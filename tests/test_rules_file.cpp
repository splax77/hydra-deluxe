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
    // Pinned: the keys Hydra 1.9.0 stopped reading when Auto was removed.
    for (const char* key : {"auto_cap_ladder", "auto_budget_s"}) {
        CAPTURE(key);
        hydra::core::Rules r;
        CHECK_NOTHROW(r = load_rules_text(std::string(key) + " = banana\n"));
        CHECK(r.fingerprint() == hydra::core::default_rules().fingerprint());
    }
}

TEST_CASE("a key that is neither live nor retired still stops the rules file") {
    CHECK_THROWS_AS(load_rules_text("auto_cap_lader = 4\n"), hydra::app::RulesFileError);
}
