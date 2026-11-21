// hydra_rules.ini's retired keys (kRetiredRulesKeys in src/app/rules_file.cpp),
// its floors, and how it words a bad line.

#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "app/ini_errors.h"
#include "app/config.h"
#include "app/rules_file.h"
#include "core/rules.h"
#include "temp_util.h"

namespace {

// Writes `text` as the whole of `file`.
void write_text(const testtemp::ScopedFile& file, const std::string& text) {
    std::ofstream f(static_cast<std::filesystem::path>(file), std::ios::trunc);
    f << text;
}

// Loads a rules file holding `text`, then deletes it.
hydra::core::Rules load_rules_text(const std::string& text) {
    const testtemp::ScopedFile file("rules_retired", ".ini");
    write_text(file, text);
    return hydra::app::load_rules_file(file);
}

}  // namespace

TEST_CASE("each retired rules key loads silently, whatever it says") {
    // Pinned: the keys in kRetiredRulesKeys (app/rules_file.cpp), which owns
    // why each one retired.
    for (const char* key : {"auto_cap_ladder", "auto_budget_s", "fill_length_measures"}) {
        CAPTURE(key);
        hydra::core::Rules r;
        CHECK_NOTHROW(r = load_rules_text(std::string(key) + " = banana\n"));
        CHECK(r.fingerprint() == hydra::core::default_rules().fingerprint());
    }
}

TEST_CASE("a key that is neither live nor retired still stops the rules file") {
    CHECK_THROWS_AS(load_rules_text("auto_cap_lader = 4\n"), hydra::app::RulesFileError);
}

// D270 call 3: unlike the settings file, which lands on the nearest edge, the
// rules file refuses a number below its floor. A value on the floor loads.
TEST_CASE("the rules file refuses below each of its five floors and reads the floor itself") {
    struct Case {
        const char* key;
        const char* below;
        const char* on_floor;
    };
    for (const Case& c : {Case{"backend_leeway_ms", "-0.5", "0"},
                          Case{"max_tied_paths", "0", "1"},
                          Case{"fill_cooldown_measures", "0", "1"},
                          Case{"fill_max_distance_beats", "-0.5", "0"},
                          Case{"fill_land_slop_beats", "-0.5", "0"}}) {
        CAPTURE(c.key);
        // "in range" is the floor's refusal, not "a number" for unreadable text.
        const std::string refusal =
            std::string(c.key) + " = \"" + c.below + "\" is not in range";
        CHECK_THROWS_WITH_AS(load_rules_text(std::string(c.key) + " = " + c.below + "\n"),
                             doctest::Contains(refusal.c_str()), hydra::app::RulesFileError);
        CHECK_NOTHROW(load_rules_text(std::string(c.key) + " = " + c.on_floor + "\n"));
    }
}


// The mirror of test_config.cpp's "a bad line refuses": each bad line is
// loaded through both the rules loader and the settings loader, and each
// refusal must be the text app/ini_errors.h builds for its own file, so the
// two files word the same mistake the same way.
TEST_CASE("a bad rules line refuses in the settings file's shapes") {
    const testtemp::ScopedFile rules_file("rules_badline", ".ini");
    const testtemp::ScopedFile settings_file("settings_badline", ".ini");
    // The rules loader names its file by the path's u8string; the settings
    // loader by the path text it was given.
    const std::string rules_path = static_cast<std::filesystem::path>(rules_file).u8string();
    const std::string& settings_path = settings_file.path;
    // Each loader's refusal of a file holding `line`, or "" when it loads.
    auto rules_refusal = [&rules_file](const std::string& line) -> std::string {
        write_text(rules_file, line + "\n");
        try {
            hydra::app::load_rules_file(rules_file);
        } catch (const hydra::app::RulesFileError& e) {
            return e.what();
        }
        return "";
    };
    auto settings_refusal = [&settings_file](const std::string& line) -> std::string {
        write_text(settings_file, line + "\n");
        try {
            hydra::app::Settings::load_file(settings_file.path);
        } catch (const hydra::app::SettingsFileError& e) {
            return e.what();
        }
        return "";
    };

    // A line with no "=" and an unknown key: the same line in both files.
    const std::string no_equals = "no_equals_sign_here";
    CHECK(rules_refusal(no_equals) == hydra::app::ini_not_key_value_text(rules_path, no_equals));
    CHECK(settings_refusal(no_equals) ==
          hydra::app::ini_not_key_value_text(settings_path, no_equals));
    CHECK(rules_refusal("unknown_key=whatever") ==
          hydra::app::ini_unknown_key_text(rules_path, "unknown_key"));
    CHECK(settings_refusal("unknown_key=whatever") ==
          hydra::app::ini_unknown_key_text(settings_path, "unknown_key"));

    // Text that is not a whole number, in a whole-number key of each file
    // (core::Rules::max_tied_paths and Settings::sp_cap are both ints).
    for (const std::string value : {"abc", "2.5"}) {
        CAPTURE(value);
        CHECK(rules_refusal("max_tied_paths=" + value) ==
              hydra::app::ini_bad_value_text(rules_path, "max_tied_paths", value,
                                             hydra::app::kWantWholeNumber));
        CHECK(settings_refusal("sp_cap=" + value) ==
              hydra::app::ini_bad_value_text(settings_path, "sp_cap", value,
                                             hydra::app::kWantWholeNumber));
    }

    // The squeeze-out rule has no settings twin. Its refusal has the shared
    // shape, and each name it offers after "is not" is one the rules file
    // takes.
    const std::string sqout = rules_refusal("sqout_rule=middle");
    const std::string shape = hydra::app::ini_bad_value_text(rules_path, "sqout_rule", "middle", "");
    REQUIRE(sqout.rfind(shape, 0) == 0);
    std::string_view offered = std::string_view(sqout).substr(shape.size());
    REQUIRE(!offered.empty());
    constexpr std::string_view kOr = " or ";
    while (!offered.empty()) {
        const size_t cut = offered.find(kOr);
        const std::string name(offered.substr(0, cut));
        CAPTURE(name);
        CHECK(hydra::core::sqout_rule_from_name(name).has_value());
        offered = cut == std::string_view::npos ? std::string_view{} : offered.substr(cut + kOr.size());
    }
}
