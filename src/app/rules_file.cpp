#include "app/rules_file.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <variant>

#include "app/config.h"
#include "app/user_messages.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "core/winstr.h"

namespace hydra::app {

namespace {

// hydra_rules.ini's retired keys (RetiredKey in app/config.h). Any other
// unknown key is an error.
constexpr RetiredKey kRetiredRulesKeys[] = {
    {"auto_cap_ladder", "1.9.0"},  // Auto was removed
    {"auto_budget_s", "1.9.0"},    // Auto was removed
};

[[noreturn]] void bad(const std::string& path, const std::string& key,
                      const std::string& value, const char* want) {
    throw RulesFileError(path + ": " + key + " = \"" + value + "\" is not " + want);
}

// A number at least f.min (above it when f.min_allowed is false).
void check_min(const std::string& path, const core::RulesField& f, const std::string& v,
               double d) {
    if (d < f.min) bad(path, f.name, v, "in range");
    if (!f.min_allowed && d == f.min) bad(path, f.name, v, "above zero");
}

// What text is a number: core/strutil.h parse_finite_number (D114 item 8).
double to_double(const std::string& path, const std::string& key, const std::string& v) {
    const std::optional<double> d = parse_finite_number(v);
    if (!d) bad(path, key, v, "a number");
    return *d;
}

int to_int(const std::string& path, const std::string& key, const std::string& v) {
    const std::optional<double> d = parse_finite_number(v);
    if (!d || *d != std::trunc(*d) || *d < std::numeric_limits<int>::min() ||
        *d > std::numeric_limits<int>::max())
        bad(path, key, v, "a whole number");
    return static_cast<int>(*d);
}

// Sets the field `f` of `r` from the file's text `v`, or throws naming it.
void set_field(const std::string& path, const core::RulesField& f, const std::string& v,
               core::Rules& r) {
    if (auto real = std::get_if<double core::Rules::*>(&f.member)) {
        const double d = to_double(path, f.name, v);
        check_min(path, f, v, d);
        r.**real = d;
    } else if (auto whole = std::get_if<int core::Rules::*>(&f.member)) {
        const int i = to_int(path, f.name, v);
        check_min(path, f, v, i);
        r.**whole = i;
    } else {
        const std::optional<core::SqOutRule> rule = core::sqout_rule_from_name(v);
        if (!rule) bad(path, f.name, v, "first_note or whole_chord");
        r.*std::get<core::SqOutRule core::Rules::*>(f.member) = *rule;
    }
}

}  // namespace

core::Rules load_rules_file(const std::filesystem::path& path) {
    core::Rules r;
    std::ifstream f(os_path(path));
    if (!f) return r;  // no file: today's rules
    const std::string where = path.u8string();

    std::string line;
    while (std::getline(f, line)) {
        const std::optional<IniPair> kv = split_ini_line(line);
        if (!kv) {
            // A blank or comment-only line is fine; any other line needs an =.
            const std::string text(ini_line_text(line));
            if (text.empty()) continue;
            throw RulesFileError(where + ": \"" + text + "\" is not key = value");
        }
        const core::RulesField* field = nullptr;
        for (const core::RulesField& candidate : core::rules_fields())
            if (kv->key == candidate.name) field = &candidate;
        // A live field wins, so a name brought back is never skipped.
        if (!field && is_retired_key(kRetiredRulesKeys, kv->key)) continue;
        if (!field) throw RulesFileError(where + ": unknown key \"" + kv->key + "\"");
        set_field(where, *field, kv->value, r);
    }
    return r;
}

std::filesystem::path default_rules_path() {
    if (!path_overrides().rules_path.empty())
        return std::filesystem::u8path(path_overrides().rules_path);
    return std::filesystem::u8path(exe_dir()) / "hydra_rules.ini";
}

int load_tool_rules(const std::optional<std::string>& path, core::Rules& rules) {
    try {
        rules = load_rules_file(path ? std::filesystem::u8path(*path) : default_rules_path());
    } catch (const RulesFileError& e) {
        std::fprintf(stderr, "%s\n", rules_file_tool_text(e).c_str());
        return 2;
    }
    return 0;
}

}  // namespace hydra::app
