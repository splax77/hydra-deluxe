// The live channel's pieces that need no GUI: the seed guard,
// testtemp::is_under_temp (tests/temp_util.h), which live mode uses to refuse
// a seed that is not a copy in the temp folder, and the JSON lines it writes
// (tests/ui/live_protocol.h), pinned as literals.

#include "doctest.h"

#include <cctype>
#include <string>

#include "temp_util.h"
#include "ui/live_protocol.h"

TEST_CASE("live reply: a command that worked") {
    CHECK(live::ok_line("") == "{\"ok\":true,\"text\":\"\"}\n");
    CHECK(live::ok_line("charts=97\n") == "{\"ok\":true,\"text\":\"charts=97\\n\"}\n");
}

TEST_CASE("live reply: a failure with a quote, a backslash and a newline in it") {
    CHECK(live::fail_line("!! failed at line 2: click \"Scan\"", "[error] C:\\x\nnext\n") ==
          "{\"ok\":false,\"error\":\"!! failed at line 2: click \\\"Scan\\\"\","
          "\"log\":\"[error] C:\\\\x\\nnext\\n\"}\n");
}

TEST_CASE("live reply: control characters are escaped, UTF-8 passes through") {
    CHECK(live::json_string("a\tb\rc\x01\x1f") == "\"a\\tb\\rc\\u0001\\u001f\"");
    CHECK(live::json_string("caf\xC3\xA9") == "\"caf\xC3\xA9\"");
}

TEST_CASE("live hello line") {
    CHECK(live::hello_line("headless", "C:\\Temp\\hydra_test_uitest_1234", 1234, 30.0) ==
          "{\"ok\":true,\"hello\":{\"mode\":\"headless\","
          "\"scratch\":\"C:\\\\Temp\\\\hydra_test_uitest_1234\",\"pid\":1234,\"timeout\":30}}\n");
    CHECK(live::hello_line("window", "s", 7, 2.5) ==
          "{\"ok\":true,\"hello\":{\"mode\":\"window\",\"scratch\":\"s\",\"pid\":7,\"timeout\":2.5}}\n");
}

TEST_CASE("is_under_temp: a file in a folder under the temp folder passes") {
    CHECK(testtemp::is_under_temp(testtemp::temp_root() + "hydra_live_1\\hydra.db"));
    CHECK(testtemp::is_under_temp(testtemp::temp_root() + "hydra.db"));
}

TEST_CASE("is_under_temp: the installed library is refused") {
    CHECK_FALSE(testtemp::is_under_temp("C:\\Program Files\\Hydra\\hydra.db"));
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
