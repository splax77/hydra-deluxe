// The one owner of Hydra's small string helpers (core/strutil). Every caller
// that used to carry its own lowercase, trim or suffix check reads these.

#include "doctest.h"

#include <optional>
#include <string>
#include <string_view>

#include "core/strutil.h"

using namespace hydra;

TEST_CASE("strutil: to_lower_ascii lowers A-Z and leaves every other byte") {
    CHECK(to_lower_ascii("Notes.MID") == "notes.mid");
    CHECK(to_lower_ascii("AbCdEF0123") == "abcdef0123");
    // UTF-8 bytes pass through untouched: "ÉTÉ" becomes "ÉtÉ".
    CHECK(to_lower_ascii("\xC3\x89T\xC3\x89") == "\xC3\x89t\xC3\x89");
    CHECK(to_lower_ascii("") == "");
}

TEST_CASE("strutil: trim strips ASCII whitespace from both ends only") {
    CHECK(trim("  key = value \r\n") == "key = value");
    CHECK(trim("\t\v\fx y\f\v\t") == "x y");
    CHECK(trim(" \t\r\n") == "");
    CHECK(trim("") == "");
    CHECK(trim("inner  space") == "inner  space");
}

TEST_CASE("strutil: trim_view trims like trim and points into the input") {
    for (const char* s : {"  key = value \r\n", "\t\v\fx y\f\v\t", " \t\r\n", "",
                          "inner  space", "x", " x "}) {
        CHECK(std::string(trim_view(s)) == trim(s));
    }
    // A view, not a copy: it lies inside the original text.
    const std::string text = "  [Song]\r\n";
    const std::string_view v = trim_view(text);
    CHECK(std::string(v) == "[Song]");
    CHECK(static_cast<const void*>(v.data()) == static_cast<const void*>(text.data() + 2));
    CHECK(trim_view("   ").empty());
}

TEST_CASE("strutil: ends_with is exact and ends_with_ci ignores ASCII case") {
    CHECK(ends_with("song.mid", ".mid"));
    CHECK(ends_with(".mid", ".mid"));
    CHECK_FALSE(ends_with("song.MID", ".mid"));
    CHECK_FALSE(ends_with("mid", ".mid"));
    CHECK(ends_with_ci("SONG.Mid", ".mid"));
    CHECK(ends_with_ci("track.OPUS", ".opus"));
    CHECK_FALSE(ends_with_ci("track.opus.bak", ".opus"));
    CHECK_FALSE(ends_with_ci("s", ".sng"));
    CHECK(ends_with_ci("x", ""));
}

TEST_CASE("strutil: the INI line splitter trims, cuts at #, splits at the first =") {
    const std::optional<IniPair> abc = split_ini_line("  a = b = c # x ");
    REQUIRE(abc.has_value());
    CHECK(abc->key == "a");
    CHECK(abc->value == "b = c");
    // The text after the first =, with any # kept, for a key whose value is
    // free text such as a folder path.
    CHECK(abc->whole_value == "b = c # x");
    CHECK_FALSE(split_ini_line("# only").has_value());
    CHECK_FALSE(split_ini_line("no equals").has_value());
    CHECK_FALSE(split_ini_line("   ").has_value());
    // A # before the = makes the whole line a comment.
    CHECK_FALSE(split_ini_line("#k=v").has_value());
    const std::optional<IniPair> empty = split_ini_line("k=");
    REQUIRE(empty.has_value());
    CHECK(empty->key == "k");
    CHECK(empty->value == "");
    // What is left of a line once the comment is cut: a reader that refuses
    // a "="-less line tells it from a blank or comment-only one by this.
    CHECK(std::string(ini_line_text("  no equals # x")) == "no equals");
    CHECK(std::string(ini_line_text("  # only")) == "");
}

TEST_CASE("strutil: parse_bool takes 0 and 1 only") {
    CHECK(parse_bool("1") == std::optional<bool>(true));
    CHECK(parse_bool("0") == std::optional<bool>(false));
    for (const char* text : {"true", "yes", "", " 1"}) {
        CAPTURE(text);
        CHECK_FALSE(parse_bool(text).has_value());
    }
}
