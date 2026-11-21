// GitHub, replayed on this PC for the updater's tests: the files saved from
// GitHub (update_reply.h, real_dir) served by the shared local server
// (loopback_server.h), with Hydra's GitHub addresses sent to it by
// net::set_address_map_for_test. Only the address is a stand-in; every step
// after it is Hydra's real network code. Also the scratch temp folder the
// updater's tests run under.
//
// No doctest here: the GUI test runner (tests/ui) uses it too. Include it
// before anything that includes <windows.h> (see loopback_server.h).

#ifndef HYDRA_TESTS_UPDATE_REPLAY_H
#define HYDRA_TESTS_UPDATE_REPLAY_H

#include "loopback_server.h"  // first: it brings in winsock2.h ahead of windows.h

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/update_release.h"  // parse_latest_release_json
#include "core/winstr.h"         // read_file_text, utf8_to_wide
#include "net/http.h"            // set_address_map_for_test
#include "temp_util.h"
#include "ui/update_job.h"       // kLatestReleaseUrl
#include "update_reply.h"

namespace update_fixtures {

// The part of an absolute URL before its path: "https://api.github.com".
inline std::string url_origin(const std::string& url) {
    const size_t scheme = url.find("://");
    if (scheme == std::string::npos) throw std::runtime_error("not an absolute URL: " + url);
    const size_t slash = url.find('/', scheme + 3);
    return slash == std::string::npos ? url : url.substr(0, slash);
}

// The path of an absolute URL, query included, as a client asks the server
// for it: "/repos/splax77/hydra-deluxe/releases/latest".
inline std::string url_path(const std::string& url) {
    const std::string origin = url_origin(url);
    return origin.size() == url.size() ? std::string("/") : url.substr(origin.size());
}

// GitHub for one test: a local server holding a release reply at the path of
// ui::kLatestReleaseUrl, and the saved installer and signature at the paths
// of that reply's own two download addresses (as app::parse_latest_release_json
// reads them). While it lives, the address map sends each of those three
// addresses' origins to the server, so any other address throws (net/http.h).
// It removes the map when it is destroyed.
class GitHubReplay {
public:
    // Serves the saved reply as GitHub sent it.
    GitHubReplay() : GitHubReplay(real_reply_text()) {}

    // Serves `reply`, which must name the saved release's assets (the saved
    // reply, or real_reply_at of a tag).
    explicit GitHubReplay(const std::string& reply)
        : release_(hydra::app::parse_latest_release_json(reply)),
          server_(routes(reply, release_)) {
        std::vector<hydra::net::AddressPair> map;
        for (const std::string& url :
             {std::string(hydra::ui::kLatestReleaseUrl), release_.installer_url, release_.sig_url}) {
            const std::string origin = url_origin(url);
            bool mapped = false;
            for (const hydra::net::AddressPair& pair : map) mapped = mapped || pair.from == origin;
            if (!mapped) map.push_back({origin, server_.url()});
        }
        hydra::net::set_address_map_for_test(std::move(map));
    }

    ~GitHubReplay() { hydra::net::set_address_map_for_test({}); }
    GitHubReplay(const GitHubReplay&) = delete;
    GitHubReplay& operator=(const GitHubReplay&) = delete;

    // The server, to count requests, change a route, or pause and release one.
    testnet::LoopbackServer& server() { return server_; }

    // The release the served reply describes, read by the production parser.
    const hydra::app::UpdateRelease& release() const { return release_; }

    // The server paths of the check, the installer and the signature.
    std::string check_route() const { return url_path(hydra::ui::kLatestReleaseUrl); }
    std::string installer_route() const { return url_path(release_.installer_url); }
    std::string signature_route() const { return url_path(release_.sig_url); }

    // The saved installer file the installer route serves.
    std::string installer_file() const { return real_asset_path(release_.installer_url); }

    // Stops the server while the map stays, so a request to GitHub's
    // addresses is really refused, as by a server that is down.
    void stop() { server_.stop(); }

private:
    static std::vector<testnet::Route> routes(const std::string& reply,
                                              const hydra::app::UpdateRelease& served) {
        std::vector<testnet::Route> out;
        out.push_back({url_path(hydra::ui::kLatestReleaseUrl), "200 OK", reply});
        out.push_back({url_path(served.installer_url), "200 OK",
                       hydra::read_file_text(real_asset_path(served.installer_url))});
        out.push_back({url_path(served.sig_url), "200 OK",
                       hydra::read_file_text(real_asset_path(served.sig_url))});
        return out;
    }

    hydra::app::UpdateRelease release_;  // first: server_ is built from it
    testnet::LoopbackServer server_;
};

// Gives this test process a scratch temp folder for its life: TMP and TEMP
// name a fresh testtemp::temp_dir(tag), so code that asks Windows for the temp
// folder, ui::update_folder among it, is answered with the scratch folder.
// At the end it puts the old values back, then deletes the folder.
class ScopedTempFolder {
public:
    explicit ScopedTempFolder(const std::string& tag) {
        testtemp::remove_dir(testtemp::temp_path(tag, ""));
        folder_ = testtemp::temp_dir(tag);
        old_tmp_ = read_variable(L"TMP");
        old_temp_ = read_variable(L"TEMP");
        const std::wstring wide = hydra::utf8_to_wide(folder_);
        if (!SetEnvironmentVariableW(L"TMP", wide.c_str()) ||
            !SetEnvironmentVariableW(L"TEMP", wide.c_str())) {
            restore();
            throw std::runtime_error("could not point TMP and TEMP at " + folder_);
        }
    }
    ~ScopedTempFolder() {
        restore();
        testtemp::remove_dir(folder_);
    }
    ScopedTempFolder(const ScopedTempFolder&) = delete;
    ScopedTempFolder& operator=(const ScopedTempFolder&) = delete;

    // The scratch folder, with no separator at its end.
    const std::string& folder() const { return folder_; }

private:
    struct Saved {
        bool present = false;
        std::wstring value;
    };

    static Saved read_variable(const wchar_t* name) {
        Saved saved;
        const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
        if (size == 0) return saved;
        std::wstring value(size, L'\0');
        const DWORD length = GetEnvironmentVariableW(name, value.data(), size);
        value.resize(length);
        saved.present = true;
        saved.value = std::move(value);
        return saved;
    }

    static void write_variable(const wchar_t* name, const Saved& saved) {
        SetEnvironmentVariableW(name, saved.present ? saved.value.c_str() : nullptr);
    }

    void restore() {
        write_variable(L"TMP", old_tmp_);
        write_variable(L"TEMP", old_temp_);
    }

    std::string folder_;
    Saved old_tmp_;
    Saved old_temp_;
};

}  // namespace update_fixtures

#endif  // HYDRA_TESTS_UPDATE_REPLAY_H
