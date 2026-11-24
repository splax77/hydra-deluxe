// Tests for app/export_name: the file names the "Export to Excel..." save
// dialog suggests, the rule that makes a player's name safe in one, and the
// export_folder setting the dialog opens in.

#include "doctest.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/export_name.h"
#include "core/winstr.h"
#include "temp_util.h"

using hydra::app::ExportDate;
using hydra::app::Settings;

TEST_CASE("the suggested export names read as the user's answer gives them") {
    const ExportDate day{2026, 11, 23};
    CHECK(hydra::app::path_report_export_name(day) == "Path report 2026-11-23.xlsx");
    CHECK(hydra::app::dm_report_export_name("Jane", day) ==
          "dmleaderboards Jane 2026-11-23.xlsx");

    // Month and day keep two digits, so the names sort by date.
    const ExportDate early{2027, 1, 5};
    CHECK(hydra::app::path_report_export_name(early) == "Path report 2027-01-05.xlsx");
}

TEST_CASE("a refused character in the player's name becomes an underscore in the file name") {
    const ExportDate day{2026, 11, 23};
    CHECK(hydra::app::dm_report_export_name("a<b>c:d\"e/f\\g|h?i*j", day) ==
          "dmleaderboards a_b_c_d_e_f_g_h_i_j 2026-11-23.xlsx");
}

TEST_CASE("a name with no refused character comes back unchanged") {
    // Spaces, dots, brackets and a non-ASCII letter (UTF-8) are all allowed.
    const std::string name = "Jane Doe. (2) [x] {y} #1 Zo\xC3\xAB";
    CHECK(hydra::app::file_name_safe(name) == name);
}

// Settles the plan's premise U3: every character from 1 to 127, made safe,
// keeps its length and names a file Windows really creates under that exact
// name. Fails if the rule misses a character Windows refuses (the create fails
// or the name comes back changed) or if it drops or adds a character.
TEST_CASE("every ASCII character, made safe, names a file Windows creates as is") {
    std::string every;
    for (int c = 1; c <= 127; ++c) every.push_back(static_cast<char>(c));
    const std::string safe = hydra::app::file_name_safe(every);
    REQUIRE(safe.size() == every.size());

    const std::string dir = testtemp::temp_dir("export_name_every_char");
    std::error_code ec;
    for (const auto& old : std::filesystem::directory_iterator(hydra::os_path(dir), ec))
        std::filesystem::remove(old.path(), ec);

    const std::string file = safe + ".txt";
    {
        std::ofstream f(hydra::os_path(hydra::join_folder(dir, file)));
        REQUIRE(f.good());
        f << "x";
    }
    std::vector<std::string> names;
    for (const auto& e : std::filesystem::directory_iterator(hydra::os_path(dir)))
        names.push_back(hydra::wide_to_utf8(e.path().filename().wstring()));
    testtemp::remove_dir(dir);

    REQUIRE(names.size() == 1);
    CHECK(names[0] == file);
}

TEST_CASE("export_folder survives a settings save and load") {
    Settings s;
    CHECK(s.export_folder.empty());
    // A # inside a folder name stays part of it (a free-text key).
    s.export_folder = "C:\\Users\\Me\\Excel #1";

    const std::string path = testtemp::temp_path("export_folder", ".ini");
    REQUIRE(s.save_file(path));
    const Settings r = Settings::load_file(path);
    std::remove(path.c_str());
    CHECK(r.export_folder == s.export_folder);
}
