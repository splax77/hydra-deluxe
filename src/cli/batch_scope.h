// Which charts a hydra_batch run reads. A run over the whole library must be
// asked for by name, so a bare `hydra_batch` never starts one.
#pragma once

#include <string>
#include <string_view>

namespace hydra::cli {

inline constexpr std::string_view kWholeLibraryFlag = "--whole-library";

// What a run's folder arguments and kWholeLibraryFlag ask for.
enum class BatchScope {
    Library,    // every folder in the app's settings
    Folders,    // just the folders given
    Refused,    // neither: a whole-library run must be asked for by name
    Ambiguous,  // both: the flag and folders contradict each other
};

inline BatchScope batch_scope(bool whole_library, bool has_folders) {
    if (whole_library) return has_folders ? BatchScope::Ambiguous : BatchScope::Library;
    return has_folders ? BatchScope::Folders : BatchScope::Refused;
}

// The one line a refused or ambiguous run prints; empty for a run that goes ahead.
inline std::string batch_scope_error(BatchScope scope) {
    const std::string flag(kWholeLibraryFlag);
    switch (scope) {
        case BatchScope::Refused:
            return "A run over the whole library needs " + flag +
                   ". Name chart folders to analyze just those.";
        case BatchScope::Ambiguous:
            return flag + " analyzes every folder in the app's settings, so it takes no folder names.";
        case BatchScope::Library:
        case BatchScope::Folders:
            break;
    }
    return {};
}

}  // namespace hydra::cli
