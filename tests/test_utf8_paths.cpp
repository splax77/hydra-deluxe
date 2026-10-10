// Non-ASCII install folders. Hydra runs from wherever the user put it, and a
// folder like C:\Users\Zoë\... is a UTF-8 path in our std::strings. The narrow
// CRT and std::fstream calls read such a path through the ANSI code page and
// miss the file, so every reader goes through core/winstr's wide calls. The
// Preview renderer's case lives with its other cases in
// test_preview_renderer.cpp.

#include "doctest.h"

#include <cstdio>
#include <stdexcept>
#include <string>

#include "app/config.h"
#include "core/winstr.h"
#include "temp_util.h"

using testtemp::non_ascii_dir;
using testtemp::remove_dir;

TEST_CASE("read_file_text reads a file under a non-ASCII folder") {
    const std::string dir = non_ascii_dir("text");
    const std::string file = dir + "\\note.txt";
    std::FILE* f = hydra::fopen_utf8(file, L"wb");
    REQUIRE(f != nullptr);
    std::fputs("hello\r\nworld", f);
    std::fclose(f);

    CHECK(hydra::read_file_text(file) == "hello\r\nworld");  // bytes as they are
    CHECK_THROWS_AS(hydra::read_file_text(dir + "\\missing.txt"), std::runtime_error);
    remove_dir(dir);
}

TEST_CASE("the settings INI saves and loads under a non-ASCII folder") {
    const std::string dir = non_ascii_dir("ini");
    const std::string ini = dir + "\\hydra_settings.ini";
    hydra::app::Settings s;
    s.depth_value = 7;
    s.dm_last_user = "111";
    REQUIRE(s.save_file(ini));
    CHECK(hydra::file_exists_utf8(ini));

    hydra::app::Settings back = hydra::app::Settings::load_file(ini);
    CHECK(back.depth_value == 7);
    CHECK(back.dm_last_user == "111");
    remove_dir(dir);
}
