// hydra_rules.ini's retired keys (kRetiredRulesKeys in src/app/rules_file.cpp).

#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "app/rules_file.h"
#include "core/rules.h"
#include "temp_util.h"

namespace {

// Loads a rules file holding `text`, then deletes it.
hydra::core::Rules load_rules_text(const std::string& text) {
    const testtemp::ScopedFile file("rules_retired", ".ini");
    {
        std::ofstream f(static_cast<std::filesystem::path>(file), std::ios::trunc);
        f << text;
    }
    return hydra::app::load_rules_file(file);
}

}  // namespace

TEST_CASE("each retired rules key loads silently, whatever it says") {
    // Pinned: the keys Hydra 1.9.0 stopped reading when Auto was removed, and
    // the one 2.8.0 stopped reading when a generated fill's length stopped
    // being a rule (D115).
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

// The mirror of test_config.cpp's "a bad line refuses": the rules file words
// each bad line in the same shapes, so one helper (app/ini_errors.h) serves
// both files.
TEST_CASE("a bad rules line refuses in the settings file's shapes") {
    const testtemp::ScopedFile file("rules_badline", ".ini");
    const std::string path = static_cast<std::filesystem::path>(file).u8string();
    auto load = [&file](const char* line) {
        {
            std::ofstream f(static_cast<std::filesystem::path>(file), std::ios::trunc);
            f << line << "\n";
        }
        return hydra::app::load_rules_file(file);
    };
    using hydra::app::RulesFileError;
    CHECK_THROWS_WITH_AS(load("no_equals_sign_here"),
                         (path + ": \"no_equals_sign_here\" is not key = value").c_str(),
                         RulesFileError);
    CHECK_THROWS_WITH_AS(load("unknown_key=whatever"),
                         (path + ": unknown key \"unknown_key\"").c_str(), RulesFileError);
    CHECK_THROWS_WITH_AS(load("max_tied_paths=abc"),
                         (path + ": max_tied_paths = \"abc\" is not a whole number").c_str(),
                         RulesFileError);
    CHECK_THROWS_WITH_AS(load("max_tied_paths=2.5"),
                         (path + ": max_tied_paths = \"2.5\" is not a whole number").c_str(),
                         RulesFileError);
    CHECK_THROWS_WITH_AS(load("sqout_rule=middle"),
                         (path + ": sqout_rule = \"middle\" is not first_note or whole_chord").c_str(),
                         RulesFileError);
}
