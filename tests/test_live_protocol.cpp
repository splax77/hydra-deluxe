// The live channel's pieces that need no GUI. For now, the seed guard:
// testtemp::is_under_temp (tests/temp_util.h), which live mode uses to refuse
// a seed that is not a copy in the temp folder.

#include "doctest.h"

#include <cctype>
#include <string>

#include "temp_util.h"

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
