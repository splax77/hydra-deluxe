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

TEST_CASE("run guard: an unquoted filter split at its dot is refused") {
    // What PowerShell passes for an unquoted -sf=*test_track_state.cpp.
    CHECK(refuse_arguments(Args{"-sf=*test_track_state", ".cpp"}) ==
          std::optional<std::string>(
              "hydra_tests: the argument \".cpp\" does not start with a dash, so it is not an option. "
              "PowerShell splits an unquoted filter at its first dot, so put quotes around the whole "
              "filter, for example \"-sf=*test_x.cpp\"."));
    CHECK(refuse_arguments(Args{"-tc=fuzz", ".seeds*"}).has_value());
}

TEST_CASE("run guard: a filter given twice is refused, however many dashes or dt- prefix it has") {
    CHECK(refuse_arguments(Args{"-sf=*single_owner*", "-sf=*test_precheck*"}) ==
          std::optional<std::string>(
              "hydra_tests: \"-sf=*single_owner*\" and \"-sf=*test_precheck*\" give the same filter twice. "
              "doctest keeps only one of them and drops the other without a word. Join the patterns with "
              "commas in one filter, or run each as its own command."));
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
    CHECK(refuse_arguments(Args{"-tc=docs match code*"}) == std::nullopt);
    // An include and an exclude of the same thing are two kinds.
    CHECK(refuse_arguments(Args{"-sf=*a*", "-sfe=*b*", "-tc=x*", "-tce=y*"}) == std::nullopt);
    // tools/check_shards.ps1's listing form.
    CHECK(refuse_arguments(Args{"-sf=*test_a.cpp", "-ltc", "--no-skip", "--reporters=xml"}) == std::nullopt);
    // An empty filter is no filter to doctest, so it repeats nothing.
    CHECK(refuse_arguments(Args{"-sf=", "-sf=*a*"}) == std::nullopt);
}

TEST_CASE("run guard: a real run whose include filter matched nothing is refused") {
    CHECK(refuse_zero_match(Args{"-sf=*no_such_file_xyz*"}, 0) ==
          std::optional<std::string>("hydra_tests: no test case matched the filters \"-sf=*no_such_file_xyz*\", "
                                     "so this run tested nothing. Check the patterns."));
    CHECK(refuse_zero_match(Args{"-tc=nothing*", "-sfe=*b*", "-s"}, 0) ==
          std::optional<std::string>("hydra_tests: no test case matched the filters \"-tc=nothing*\" "
                                     "\"-sfe=*b*\", so this run tested nothing. Check the patterns."));
    CHECK(refuse_zero_match(Args{"--dt-test-suite=none"}, 0).has_value());
}

TEST_CASE("run guard: exclude-only runs and matched runs pass the zero check") {
    CHECK(refuse_zero_match(Args{"-sf=*test_a.cpp"}, 22) == std::nullopt);
    CHECK(refuse_zero_match(Args{"-sfe=*a*,*b*"}, 0) == std::nullopt);
    // A subcase filter does not choose test cases.
    CHECK(refuse_zero_match(Args{"-sc=nothing*"}, 0) == std::nullopt);
    // With no include filter, no pattern can have missed.
    CHECK(refuse_zero_match(Args{}, 0) == std::nullopt);
}
