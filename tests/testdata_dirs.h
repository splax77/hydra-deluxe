// Where the checked-in test data lives. This is the one place a test reads
// the two folder macros CMakeLists.txt passes in, so a test that needs the
// chart corpus or a fixture calls these instead. corpus_util.h includes it;
// a header that must stay light (audio_util.h, scratch_settings.h) includes
// it alone.

#ifndef HYDRA_TESTS_TESTDATA_DIRS_H
#define HYDRA_TESTS_TESTDATA_DIRS_H

#include <string>

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

namespace corpus {

// testdata/input, the chart corpus.
inline std::string root() { return HYDRA_INPUT_DIR; }

// testdata, the fixtures folder that holds the corpus. A target that leaves
// HYDRA_TESTDATA_DIR out (CMakeLists.txt says which) gets no testdata_root(),
// so a caller there fails to compile.
#ifdef HYDRA_TESTDATA_DIR
inline std::string testdata_root() { return HYDRA_TESTDATA_DIR; }

// resource, the repo's fonts and icons folder, for the tests that set ImGui
// up. It sits beside testdata in the repo, so it hangs off testdata_root().
inline std::string resource_root() { return testdata_root() + "/../resource"; }
#endif

}  // namespace corpus

#endif  // HYDRA_TESTS_TESTDATA_DIRS_H
