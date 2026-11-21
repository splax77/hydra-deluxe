// hydra_tests' command-line guard (tests/run_guard.h), on literal argument
// lists as PowerShell hands them over.
#include "doctest.h"

#include <optional>
#include <string>
#include <vector>

#include "run_guard.h"

using run_guard::refuse_arguments;
using run_guard::refuse_zero_match;
using Args = std::vector<std::string>;

namespace {

// Whether a refusal was given and its text holds `part`. The wording itself
// is run_guard.h's; a check names only what the refusal must point at.
bool refusal_names(const std::optional<std::string>& refusal, const std::string& part) {
    return refusal.has_value() && refusal->find(part) != std::string::npos;
}

// An argument as a refusal quotes it.
std::string quoted(const std::string& arg) { return "\"" + arg + "\""; }

}  // namespace

TEST_CASE("run guard: an unquoted filter split at its dot is refused") {
    // What PowerShell passes for an unquoted -sf=*test_track_state.cpp.
    const std::optional<std::string> refusal = refuse_arguments(Args{"-sf=*test_track_state", ".cpp"});
    REQUIRE(refusal.has_value());
    // It names the stray piece and tells the user to quote the filter.
    CHECK(refusal_names(refusal, quoted(".cpp")));
    CHECK(refusal_names(refusal, "quote"));
    CHECK(refusal_names(refuse_arguments(Args{"-tc=fuzz", ".seeds*"}), quoted(".seeds*")));
}

TEST_CASE("run guard: a filter given twice is refused, however many dashes or dt- prefix it has") {
    const std::optional<std::string> refusal = refuse_arguments(Args{"-sf=*single_owner*", "-sf=*test_precheck*"});
    REQUIRE(refusal.has_value());
    // It names both arguments, so the user sees which two collide.
    CHECK(refusal_names(refusal, quoted("-sf=*single_owner*")));
    CHECK(refusal_names(refusal, quoted("-sf=*test_precheck*")));
    CHECK(refuse_arguments(Args{"-sf=*a.cpp", "--dt-sf=*b.cpp"}).has_value());
    CHECK(refuse_arguments(Args{"--tc=x*", "-s", "-dt-tc=y*"}).has_value());
    CHECK(refuse_arguments(Args{"--source-file=*a.cpp", "-source-file=*b.cpp"}).has_value());
}

TEST_CASE("run guard: two spellings of one filter are both kept by doctest, so they pass") {
    // doctest reads "sf" and "source-file" separately and joins their patterns.
    CHECK(refuse_arguments(Args{"-sf=*a.cpp", "--source-file=*b.cpp"}) == std::nullopt);
    CHECK(refuse_arguments(Args{"-sf=*a.cpp", "--dt-source-file=*b.cpp"}) == std::nullopt);
    CHECK(refuse_arguments(Args{"--test-case=x*", "-s", "-dt-tc=y*"}) == std::nullopt);
}

TEST_CASE("run guard: well-formed command lines pass the argument check") {
    CHECK(refuse_arguments(Args{}) == std::nullopt);
    CHECK(refuse_arguments(Args{"-sf=*test_a.cpp,*test_b.cpp"}) == std::nullopt);
    CHECK(refuse_arguments(Args{"-tc=code lexer*"}) == std::nullopt);
    // An include and an exclude of the same thing are two kinds.
    CHECK(refuse_arguments(Args{"-sf=*a*", "-sfe=*b*", "-tc=x*", "-tce=y*"}) == std::nullopt);
    // tools/check_shards.ps1's listing form.
    CHECK(refuse_arguments(Args{"-sf=*test_a.cpp", "-ltc", "--no-skip", "--reporters=xml"}) == std::nullopt);
    // An empty filter is no filter to doctest, so it repeats nothing.
    CHECK(refuse_arguments(Args{"-sf=", "-sf=*a*"}) == std::nullopt);
}

TEST_CASE("run guard: a real run whose include filter matched nothing is refused") {
    CHECK(refusal_names(refuse_zero_match(Args{"-sf=*no_such_file_xyz*"}, 0), quoted("-sf=*no_such_file_xyz*")));
    // It names every filter given, include and exclude, and not a flag that
    // filters nothing.
    const std::optional<std::string> refusal = refuse_zero_match(Args{"-tc=nothing*", "-sfe=*b*", "-s"}, 0);
    REQUIRE(refusal.has_value());
    CHECK(refusal_names(refusal, quoted("-tc=nothing*")));
    CHECK(refusal_names(refusal, quoted("-sfe=*b*")));
    CHECK_FALSE(refusal_names(refusal, quoted("-s")));
    CHECK(refusal_names(refuse_zero_match(Args{"--dt-test-suite=none"}, 0), quoted("--dt-test-suite=none")));
}

TEST_CASE("run guard: exclude-only runs and matched runs pass the zero check") {
    CHECK(refuse_zero_match(Args{"-sf=*test_a.cpp"}, 22) == std::nullopt);
    CHECK(refuse_zero_match(Args{"-sfe=*a*,*b*"}, 0) == std::nullopt);
    // A subcase filter does not choose test cases.
    CHECK(refuse_zero_match(Args{"-sc=nothing*"}, 0) == std::nullopt);
    // With no include filter, no pattern can have missed.
    CHECK(refuse_zero_match(Args{}, 0) == std::nullopt);
}
