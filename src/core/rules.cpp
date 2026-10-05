#include "core/rules.h"

#include <cstdio>
#include <string>

namespace hydra::core {

namespace {

void add_line(std::string& out, const char* key, double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%s=%.17g\n", key, v);
    out += buf;
}

uint64_t fnv1a64(const std::string& s) {
    uint64_t h = 0xcbf29ce484222325ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 0x100000001b3ull;
    }
    return h;
}

}  // namespace

namespace {

// Every field that can change a run's answer, one line each, in
// rules_fields() order. The text is frozen: changing it makes every stored
// result Stale (docs/adr/0014). A whole number is written in the same %.17g
// form as a fraction, as it always was.
std::string fixed_cap_text(const Rules& r) {
    std::string text;
    for (const RulesField& f : rules_fields()) {
        if (auto real = std::get_if<double Rules::*>(&f.member)) {
            add_line(text, f.name, r.**real);
        } else if (auto whole = std::get_if<int Rules::*>(&f.member)) {
            add_line(text, f.name, static_cast<double>(r.**whole));
        } else {
            const auto sqout = std::get<SqOutRule Rules::*>(f.member);
            text += std::string(f.name) + "=" + sqout_rule_name(r.*sqout) + "\n";
        }
    }
    return text;
}

// 0 is reserved for "no usable rules"; a hash that lands on it moves off.
uint64_t hash_rules_text(const std::string& text) {
    const uint64_t h = fnv1a64(text);
    return h == kNoRulesFingerprint ? 1 : h;
}

}  // namespace

uint64_t Rules::fingerprint() const { return hash_rules_text(fixed_cap_text(*this)); }

uint64_t Rules::retired_auto_fingerprint() const {
    // Byte for byte what 1.8.4's auto_fingerprint() hashed under the default
    // ladder. Pinned by the test "rules: the retired Auto fingerprint is what
    // Hydra 1.8.4 stamped".
    return hash_rules_text(fixed_cap_text(*this) +
                           "auto_cap_ladder=16,32,64,128,256,512,\n");
}

const Rules& default_rules() {
    static const Rules rules;
    return rules;
}

const char* sqout_rule_name(SqOutRule rule) {
    return rule == SqOutRule::WholeChord ? "whole_chord" : "first_note";
}

std::optional<SqOutRule> sqout_rule_from_name(std::string_view name) {
    for (SqOutRule rule : {SqOutRule::FirstNote, SqOutRule::WholeChord})
        if (name == sqout_rule_name(rule)) return rule;
    return std::nullopt;
}

const std::vector<RulesField>& rules_fields() {
    // The order is the fingerprint's line order, frozen with its text.
    static const std::vector<RulesField> fields = {
        {"backend_leeway_ms", &Rules::backend_leeway_ms, 0.0},
        {"sqout_rule", &Rules::sqout_rule},
        {"max_tied_paths", &Rules::max_tied_paths, 1.0},
        {"fill_cooldown_measures", &Rules::fill_cooldown_measures, 1.0},
        {"fill_max_distance_beats", &Rules::fill_max_distance_beats, 0.0},
        // Must be above zero, as hydra_rules.ini always required.
        {"fill_length_measures", &Rules::fill_length_measures, 0.0, false},
        {"fill_land_slop_beats", &Rules::fill_land_slop_beats, 0.0},
    };
    return fields;
}

const RulesStamp& default_stamp() {
    static const RulesStamp stamp = RulesStamp::of(default_rules());
    return stamp;
}

}  // namespace hydra::core
