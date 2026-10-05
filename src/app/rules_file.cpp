#include "app/rules_file.h"

#include <fstream>
#include <string>
#include <variant>

#include "app/config.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "core/winstr.h"

namespace hydra::app {

namespace {

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

double to_double(const std::string& path, const std::string& key, const std::string& v) {
    size_t used = 0;
    double d = 0;
    try { d = std::stod(v, &used); } catch (...) { bad(path, key, v, "a number"); }
    if (used != v.size()) bad(path, key, v, "a number");
    return d;
}

int to_int(const std::string& path, const std::string& key, const std::string& v) {
    size_t used = 0;
    int i = 0;
    try { i = std::stoi(v, &used); } catch (...) { bad(path, key, v, "a whole number"); }
    if (used != v.size()) bad(path, key, v, "a whole number");
    return i;
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
        // Auto's two keys (Auto was removed 2026-09-27). A file that still
        // sets them keeps loading; whatever they say is ignored.
        if (kv->key == "auto_cap_ladder" || kv->key == "auto_budget_s") continue;
        const core::RulesField* field = nullptr;
        for (const core::RulesField& candidate : core::rules_fields())
            if (kv->key == candidate.name) field = &candidate;
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

}  // namespace hydra::app
