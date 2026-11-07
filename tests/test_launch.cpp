// Starting another program (app/launch.h): how a command line is quoted, where
// the Path Builder's exe is looked for, and the installer's shortcut to it.
#include "doctest.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/launch.h"
#include "core/winstr.h"
#include "scoped_hook.h"
#include "source_tree.h"
#include "temp_util.h"

using hydra::app::command_line;
using hydra::app::quote_argument;

namespace {

// The arguments the started program reads back from `cmd`, through the split
// every Hydra exe reads its own command line with.
std::vector<std::string> read_back(const std::wstring& cmd) {
    return hydra::split_command_line_utf8(cmd);
}

}  // namespace

TEST_CASE("launch: a path with spaces is one quoted argument") {
    const std::wstring exe = L"C:\\Program Files\\Hydra\\HydraPathBuilder.exe";
    const std::wstring chart = L"C:\\Songs\\Green Day - Burnout\\notes.chart";
    const std::wstring cmd = command_line(exe, {chart});
    CHECK(cmd ==
          L"\"C:\\Program Files\\Hydra\\HydraPathBuilder.exe\" "
          L"\"C:\\Songs\\Green Day - Burnout\\notes.chart\"");
    CHECK(read_back(cmd) ==
          std::vector<std::string>{"C:\\Program Files\\Hydra\\HydraPathBuilder.exe",
                                   "C:\\Songs\\Green Day - Burnout\\notes.chart"});
}

TEST_CASE("launch: quotes and backslashes inside an argument come back whole") {
    // A quote inside, backslashes before a quote, and backslashes at the end:
    // the three places the split treats a backslash specially.
    CHECK(quote_argument(L"say \"hi\"") == L"\"say \\\"hi\\\"\"");
    CHECK(quote_argument(L"a\\\"b") == L"\"a\\\\\\\"b\"");
    CHECK(quote_argument(L"C:\\Songs\\") == L"\"C:\\Songs\\\\\"");
    CHECK(quote_argument(L"a\\b") == L"\"a\\b\"");
    CHECK(quote_argument(L"") == L"\"\"");

    const std::vector<std::wstring> args = {L"say \"hi\"", L"a\\\"b", L"C:\\Songs\\", L"a\\b",
                                            L"", L"Zo\u00eb"};
    const std::vector<std::string> got = read_back(command_line(L"C:\\x.exe", args));
    REQUIRE(got.size() == args.size() + 1);
    CHECK(got[0] == "C:\\x.exe");
    for (size_t i = 0; i < args.size(); ++i) CHECK(got[i + 1] == hydra::wide_to_utf8(args[i]));
}

TEST_CASE("launch: a missing builder exe has no path, and launching it says why") {
    // Empty: a crashed earlier run may have left the file behind.
    testtemp::remove_dir(testtemp::temp_dir("launch_sibling"));
    const std::string dir = testtemp::temp_dir("launch_sibling");
    const std::string exe = hydra::join_folder(dir, hydra::app::kPathBuilderExe);
    ScopedHook seam(hydra::app::set_sibling_dir_for_test, std::optional<std::string>(dir));

    CHECK_FALSE(hydra::app::sibling_exe_path(hydra::app::kPathBuilderExe).has_value());
    CHECK_THROWS_AS(hydra::app::launch_sibling(hydra::app::kPathBuilderExe, {"notes.chart"}),
                    std::runtime_error);

    // A folder with the exe's name is not the exe.
    std::filesystem::create_directory(hydra::os_path(exe));
    CHECK_FALSE(hydra::app::sibling_exe_path(hydra::app::kPathBuilderExe).has_value());
    std::filesystem::remove(hydra::os_path(exe));

    // An empty file is enough for the button: it only asks whether the file is there.
    testtemp::write_bytes(exe, {});
    const std::optional<std::string> found = hydra::app::sibling_exe_path(hydra::app::kPathBuilderExe);
    REQUIRE(found.has_value());
    CHECK(*found == exe);
    testtemp::remove_dir(dir);
}

TEST_CASE("installer: the builder has a Start Menu shortcut") {
    const std::string iss =
        hydra::read_file_text((sourcetree::root() / "installer" / "hydra.iss").u8string());
    const size_t icons = iss.find("\n[Icons]");
    REQUIRE(icons != std::string::npos);
    const size_t next = iss.find("\n[", icons + 1);
    const std::string section = iss.substr(icons, next == std::string::npos ? std::string::npos
                                                                            : next - icons);
    const std::string target =
        std::string("Filename: \"{app}\\") + hydra::app::kPathBuilderExe + "\"";
    const size_t at = section.find(target);
    REQUIRE(at != std::string::npos);
    const size_t line_start = section.rfind('\n', at) + 1;
    const size_t line_end = section.find('\n', at);
    const std::string line = section.substr(line_start, line_end - line_start);
    // The user's answer of 2026-10-29: the builder's taskbar button is its own.
    CHECK(line.find("AppUserModelID") == std::string::npos);
    CHECK(line.find("Name: \"{autoprograms}\\{#HYDRA_APP_NAME} Path Builder\"") == 0);
}
