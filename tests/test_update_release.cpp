// Tests for the updater's first pieces: the version rule (core/app_version.h),
// reading GitHub's releases/latest reply (app/update_release) with its
// refusals, the refusal's plain sentence, and the installed-exe rule
// (app::installed_by_setup). Plan: docs/superpowers/plans/2026-10-22-auto-updater.md,
// task T2a, criteria B1 to B5.

#include "doctest.h"

#include <filesystem>
#include <optional>
#include <string>

#include "app/config.h"
#include "app/update_release.h"
#include "app/user_messages.h"
#include "core/app_version.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "json.hpp"
#include "scoped_hook.h"
#include "source_tree.h"
#include "testdata_dirs.h"

using hydra::ErrorKind;
using hydra::KindedError;
using hydra::core::AppVersion;
using hydra::core::parse_version;
using json = nlohmann::json;

namespace {

// The real v2.10.2 reply, with a fake signature asset added.
json fixture() {
    return json::parse(hydra::read_file_text(corpus::testdata_root() + "/update/latest.json"));
}

// The error parse_latest_release_json throws for `body`; fails the test when
// it throws nothing or something without a kind.
std::optional<KindedError> refusal(const std::string& body) {
    try {
        hydra::app::parse_latest_release_json(body);
    } catch (const KindedError& e) {
        return e;
    }
    return std::nullopt;
}

void check_refused(const json& reply, const char* why) {
    CAPTURE(why);
    const std::optional<KindedError> e = refusal(reply.dump());
    REQUIRE(e.has_value());
    CHECK(e->kind() == ErrorKind::UpdateUnverified);
}

// Index of the asset named `name` in the fixture.
size_t asset_index(const json& reply, const std::string& name) {
    for (size_t i = 0; i < reply["assets"].size(); ++i)
        if (reply["assets"][i]["name"] == name) return i;
    FAIL("no asset " << name);
    return 0;
}

constexpr const char* kInstaller = "HydraDeluxe-2.10.2-setup.exe";
constexpr const char* kSignature = "HydraDeluxe-2.10.2-setup.exe.sig";

}  // namespace

TEST_CASE("parse_version reads three whole numbers, with or without a leading v") {
    const AppVersion want{2, 10, 2};
    CHECK(parse_version("v2.10.2") == std::optional<AppVersion>(want));
    CHECK(parse_version("2.10.2") == std::optional<AppVersion>(want));
    CHECK(parse_version("0.0.0") == std::optional<AppVersion>(AppVersion{0, 0, 0}));

    CHECK_FALSE(parse_version("2.10").has_value());
    CHECK_FALSE(parse_version("abc").has_value());
    CHECK_FALSE(parse_version("").has_value());
    CHECK_FALSE(parse_version("v").has_value());
    CHECK_FALSE(parse_version("2.10.2.1").has_value());
    CHECK_FALSE(parse_version("2.10.2-beta").has_value());
    CHECK_FALSE(parse_version("2..2").has_value());
    CHECK_FALSE(parse_version("V2.10.2").has_value());
    CHECK_FALSE(parse_version("-2.10.2").has_value());
    CHECK_FALSE(parse_version(" 2.10.2").has_value());
    CHECK_FALSE(parse_version("2.10.99999999999").has_value());
}

TEST_CASE("a version is newer only when it is strictly greater") {
    const AppVersion older = *parse_version("2.9.1");
    const AppVersion newer = *parse_version("2.10.2");
    CHECK(older < newer);
    CHECK_FALSE(newer < older);
    CHECK_FALSE(newer < *parse_version("v2.10.2"));  // equal is not less
    CHECK(hydra::core::is_newer_version(newer, older));
    CHECK_FALSE(hydra::core::is_newer_version(older, newer));
    CHECK_FALSE(hydra::core::is_newer_version(newer, newer));
    CHECK(*parse_version("3.0.0") < *parse_version("10.0.0"));
    CHECK(*parse_version("2.10.2") < *parse_version("2.10.10"));
}

TEST_CASE("the running version parses through parse_version") {
    CHECK(parse_version(HYDRA_VERSION).has_value());
    CHECK(hydra::core::version_text(hydra::core::running_version()) == HYDRA_VERSION);
    CHECK(hydra::core::version_text(*parse_version("v2.10.2")) == "2.10.2");
}

TEST_CASE("the v2.10.2 reply reads into the six fields") {
    const hydra::app::UpdateRelease r = hydra::app::parse_latest_release_json(fixture().dump());
    CHECK(r.version == AppVersion{2, 10, 2});
    CHECK(r.notes == fixture()["body"].get<std::string>());
    CHECK_FALSE(r.notes.empty());
    CHECK(r.installer_url ==
          "https://github.com/splax77/hydra-deluxe/releases/download/v2.10.2/"
          "HydraDeluxe-2.10.2-setup.exe");
    CHECK(r.installer_size == 33241389);
    CHECK(r.digest == "86f386c3ecd13f835930cff691e0c3986fb264cf5313500991a4bddb122f9963");
    CHECK(r.sig_url ==
          "https://github.com/splax77/hydra-deluxe/releases/download/v2.10.2/"
          "HydraDeluxe-2.10.2-setup.exe.sig");
}

TEST_CASE("a reply with no notes reads with empty notes") {
    json reply = fixture();
    reply.erase("body");
    CHECK(hydra::app::parse_latest_release_json(reply.dump()).notes.empty());
    reply["body"] = nullptr;
    CHECK(hydra::app::parse_latest_release_json(reply.dump()).notes.empty());
}

TEST_CASE("a reply that lacks a piece the update needs is refused") {
    {
        json reply = fixture();
        reply["assets"].erase(asset_index(reply, kInstaller));
        check_refused(reply, "no -setup.exe asset");
    }
    {
        json reply = fixture();
        json second = reply["assets"][asset_index(reply, kInstaller)];
        second["name"] = "HydraDeluxe-2.10.2-other-setup.exe";
        reply["assets"].push_back(second);
        check_refused(reply, "two -setup.exe assets");
    }
    {
        json reply = fixture();
        reply["assets"].erase(asset_index(reply, kSignature));
        check_refused(reply, "no .sig asset");
    }
    {
        json reply = fixture();
        json second = reply["assets"][asset_index(reply, kSignature)];
        second["name"] = "HydraDeluxe-2.10.2-other-setup.exe.sig";
        reply["assets"].push_back(second);
        check_refused(reply, "two .sig assets");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)].erase("digest");
        check_refused(reply, "no digest");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)]["digest"] = nullptr;
        check_refused(reply, "a null digest");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)]["digest"] =
            "md5:0123456789abcdef0123456789abcdef";
        check_refused(reply, "a digest not starting sha256:");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)]["digest"] = "sha256:86f386c3";
        check_refused(reply, "a sha256 digest that is not 64 hex digits");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)].erase("size");
        check_refused(reply, "no size");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kInstaller)].erase("browser_download_url");
        check_refused(reply, "no installer address");
    }
    {
        json reply = fixture();
        reply["assets"][asset_index(reply, kSignature)].erase("browser_download_url");
        check_refused(reply, "no signature address");
    }
    {
        json reply = fixture();
        reply["tag_name"] = "latest";
        check_refused(reply, "a tag that does not parse");
    }
    {
        json reply = fixture();
        reply.erase("tag_name");
        check_refused(reply, "no tag");
    }
    {
        json reply = fixture();
        reply.erase("assets");
        check_refused(reply, "no asset list");
    }
    check_refused(json::array(), "a JSON array");

    const std::optional<KindedError> e = refusal("<html>rate limited</html>");
    REQUIRE(e.has_value());
    CHECK(e->kind() == ErrorKind::UpdateUnverified);
}

TEST_CASE("a refused update reads as decision 9's sentence, the raw text as its detail") {
    json reply = fixture();
    reply["assets"].erase(asset_index(reply, kSignature));
    const std::optional<KindedError> e = refusal(reply.dump());
    REQUIRE(e.has_value());
    CHECK(hydra::app::plain_error(*e) ==
          "Hydra couldn't verify the update it downloaded, so it was not installed. Try again "
          "later, or download the release from GitHub yourself.");
    CHECK(hydra::app::plain_error_detail(*e) == e->what());
    CHECK(std::string(e->what()) == "the release on GitHub has no asset named *-setup.exe.sig");
}

TEST_CASE("installed_by_setup is false for the test runner and follows the test seam") {
    // The test runner lives in the build folder, which no installer registered.
    CHECK_FALSE(hydra::app::installed_by_setup());
    {
        ScopedHook installed(hydra::app::set_installed_for_test, std::optional<bool>(true));
        CHECK(hydra::app::installed_by_setup());
    }
    CHECK_FALSE(hydra::app::installed_by_setup());
}

TEST_CASE("the single-owner rows name the installed-exe and version-compare owners") {
    const auto owner_of = [](const char* file, const char* symbol) {
        const std::filesystem::path path = sourcetree::root() / "tests" / "single_owner" / "rules" / file;
        const json rows = json::parse(hydra::read_file_text(path.u8string()));
        int found = 0;
        for (const json& row : rows["rules"])
            if (row["owner_symbol"] == symbol) ++found;
        return found;
    };
    CHECK(owner_of("app.json", "installed_by_setup") == 1);
    CHECK(owner_of("core.json", "parse_version") == 1);
}
