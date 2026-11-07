// Runs a test as another of Hydra's programs (app::set_program) and puts the
// previous one back on every exit path, so a failed REQUIRE can't leave the
// next test speaking as the Path Builder.

#ifndef HYDRA_TESTS_PROGRAM_SCOPE_H
#define HYDRA_TESTS_PROGRAM_SCOPE_H

#include "app/user_messages.h"

class ProgramScope {
public:
    explicit ProgramScope(hydra::app::Program program)
        : previous_(hydra::app::current_program()) {
        hydra::app::set_program(program);
    }
    ~ProgramScope() { hydra::app::set_program(previous_); }
    ProgramScope(const ProgramScope&) = delete;
    ProgramScope& operator=(const ProgramScope&) = delete;

private:
    hydra::app::Program previous_;
};

#endif  // HYDRA_TESTS_PROGRAM_SCOPE_H
