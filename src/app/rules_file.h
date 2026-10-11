// hydra_rules.ini: the user's rule choices (core/rules.h) as flat
// "key = value" lines. No [section] header. A # starts a comment anywhere on
// a line. A missing file or key keeps the default. A bad value or an unknown
// key is an error that names the key, so a typo can never silently run the
// defaults.

#ifndef HYDRA_APP_RULES_FILE_H
#define HYDRA_APP_RULES_FILE_H

#include <filesystem>
#include <optional>
#include <string>

#include "core/error_kind.h"
#include "core/rules.h"

namespace hydra::app {

// A line of hydra_rules.ini that can't be read. Its text names the file, the
// line and the key; its kind is RulesFile.
struct RulesFileError : KindedError {
    explicit RulesFileError(const std::string& what) : KindedError(ErrorKind::RulesFile, what) {}
};

core::Rules load_rules_file(const std::filesystem::path& path);

// exe_dir()\hydra_rules.ini, or PathOverrides::rules_path when set.
std::filesystem::path default_rules_path();

// How a command-line tool loads its rules: from `path` (its --rules) when
// given, else default_rules_path(). On success it fills `rules` and returns 0.
// A bad file prints the loader's line on stderr and returns the tool's exit
// code. The app does not call this: it keeps running
// with analysis off instead (AppState::rules_error).
int load_tool_rules(const std::optional<std::string>& path, core::Rules& rules);

}  // namespace hydra::app

#endif  // HYDRA_APP_RULES_FILE_H
