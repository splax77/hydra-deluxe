// The live channel's pieces that need no GUI: the seed guard,
// testtemp::is_under_temp (tests/temp_util.h), which live mode uses to refuse
// a seed that is not a copy in the temp folder, and the JSON lines it writes
// (tests/ui/live_protocol.h), parsed back the way the reader
// (tools/hydra_ui_mcp.py) reads them. The real hello line is checked from a
// real run by tools/test_hydra_ui_mcp.py, test_the_real_hello_line.

#include "doctest.h"

#include <cctype>
#include <string>

#include "json.hpp"
#include "scratch_paths.h"
#include "temp_util.h"
#include "ui/live_protocol.h"

namespace {

// One wire line parsed back as JSON. A line is one object that ends in its
// only newline, so the reader never has to guess where an answer ends.
nlohmann::json parse_line(const std::string& line) {
    REQUIRE(!line.empty());
    REQUIRE(line.find('\n') == line.size() - 1);
    return nlohmann::json::parse(line);
}

// A reply line holds exactly ok, text and timeout, and reads back as the
// values that went in.
void check_reply(const std::string& line, bool ok, const std::string& text, double timeout) {
    INFO(line);
    const nlohmann::json reply = parse_line(line);
    REQUIRE(reply.is_object());
    CHECK(reply.size() == 3u);
    CHECK(reply.at("ok").get<bool>() == ok);
    CHECK(reply.at("text").get<std::string>() == text);
    CHECK(reply.at("timeout").get<double>() == timeout);
}

}  // namespace

TEST_CASE("live reply: a command that worked") {
    // A command that produced nothing answers "ok".
    check_reply(live::ok_line("", 30.0), true, "ok", 30.0);
    check_reply(live::ok_line("charts=97\n", 2.5), true, "charts=97\n", 2.5);
}

TEST_CASE("live reply: the timeout reads back as the exact number") {
    for (const double seconds : {12.345678901, 0.1, 0.0, 100000.0, 1e-05, 1e22}) {
        INFO(seconds);
        check_reply(live::ok_line("", seconds), true, "ok", seconds);
    }
}

TEST_CASE("live reply: a failure with a quote, a backslash and a newline in it") {
    const std::string error = "failed: click \"Scan\"";
    const std::string log = "[error] C:\\x\nnext\n";
    check_reply(live::fail_line(error, log, 90.0), false, error + "\n" + log, 90.0);
}

TEST_CASE("live reply: a failure with no log lines") {
    const std::string error = "wait 99 is longer than the 30 s timeout in force";
    check_reply(live::fail_line(error, "", 30.0), false, error + "\n", 30.0);
}

TEST_CASE("live reply: control characters are escaped, UTF-8 passes through") {
    const std::string controls = "a\tb\rc\x01\x1f";
    const std::string quoted = live::json_string(controls);
    CHECK(parse_line(quoted + "\n").get<std::string>() == controls);
    for (const char ch : quoted) CHECK(static_cast<unsigned char>(ch) >= 0x20);

    const std::string accent = "caf\xC3\xA9";
    const std::string quoted_accent = live::json_string(accent);
    CHECK(parse_line(quoted_accent + "\n").get<std::string>() == accent);
    CHECK(quoted_accent.find(accent) != std::string::npos);
}

TEST_CASE("is_under_temp: a file in a folder under the temp folder passes") {
    CHECK(testtemp::is_under_temp(testtemp::temp_root() + "hydra_live_1\\hydra.db"));
    CHECK(testtemp::is_under_temp(testtemp::temp_root() + "hydra.db"));
}

TEST_CASE("is_under_temp: the library beside the exe is refused") {
    // With no override, app::db_path() is the real library file next to the exe.
    ScopedPathOverrides real_paths([](hydra::app::PathOverrides& o) { o.db_path.clear(); });
    CHECK_FALSE(testtemp::is_under_temp(hydra::app::db_path()));
}

TEST_CASE("is_under_temp: the temp folder itself, an empty path and a look-alike are refused") {
    const std::string root = testtemp::temp_root();
    CHECK_FALSE(testtemp::is_under_temp(root));
    CHECK_FALSE(testtemp::is_under_temp(""));
    // "...\Temp2\hydra.db" starts with the temp folder's letters but is not in it.
    CHECK_FALSE(testtemp::is_under_temp(root.substr(0, root.size() - 1) + "2\\hydra.db"));
}

TEST_CASE("is_under_temp: a path that climbs out with .. is refused") {
    CHECK_FALSE(testtemp::is_under_temp(testtemp::temp_root() + "hydra_live_1\\..\\..\\hydra.db"));
    CHECK(testtemp::is_under_temp(testtemp::temp_root() + "hydra_live_1\\..\\hydra.db"));
}

TEST_CASE("is_under_temp: letter case does not matter") {
    std::string upper = testtemp::temp_root() + "hydra_live_1\\hydra.db";
    for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    CHECK(testtemp::is_under_temp(upper));
}
