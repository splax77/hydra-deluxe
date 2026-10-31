// The Path Builder's build-a-whole-path test: one GUI test that builds a path
// by clicking the screen's own widgets. Task PB-6 of
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md fills it in.

// First: its Windows headers come before the test engine's, which undefine
// their Yield() macro (uitest_engine.h says why).
#include "pbtest_harness.h"

namespace pbtest {

void register_build_tests(Harness& h) { (void)h; }

}  // namespace pbtest
