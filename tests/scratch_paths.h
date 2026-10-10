// Scoped guards for a test that points the app's paths (app::path_overrides)
// somewhere else for a while. The guard puts the earlier paths back when the test's
// scope ends, even when a REQUIRE throws, so one test's paths never leak into
// the next. The scratch file names come from temp_util.h.

#ifndef HYDRA_TESTS_SCRATCH_PATHS_H
#define HYDRA_TESTS_SCRATCH_PATHS_H

#include <cstdio>
#include <string>

#include "app/config.h"
#include "temp_util.h"

// Applies `edit` to a copy of the process's path overrides and sets the
// result; the destructor sets the earlier overrides back.
struct ScopedPathOverrides {
    hydra::app::PathOverrides previous;

    template <class Edit>
    explicit ScopedPathOverrides(Edit edit) : previous(hydra::app::path_overrides()) {
        hydra::app::PathOverrides next = previous;
        edit(next);
        hydra::app::set_path_overrides(next);
    }
    ~ScopedPathOverrides() { hydra::app::set_path_overrides(previous); }

    ScopedPathOverrides(const ScopedPathOverrides&) = delete;
    ScopedPathOverrides& operator=(const ScopedPathOverrides&) = delete;
};

// Points app::ini_path(), db_path() and the rules path at scratch files for
// one test, and deletes those files before and after. commit_settings writes
// the INI through Settings::save(), so without this a test would overwrite the
// developer's real hydra_settings.ini.
struct ScratchPaths {
    std::string ini;
    std::string db;
    std::string rules;
    ScopedPathOverrides overrides;  // declared last: built after the names, undone after the files go

    explicit ScratchPaths(const std::string& tag)
        : ini(testtemp::temp_path(tag, ".ini")),
          db(testtemp::temp_path(tag, ".db")),
          rules(testtemp::temp_path(tag, "_rules.ini")),
          overrides([this](hydra::app::PathOverrides& o) {
              o.ini_path = ini;
              o.db_path = db;
              o.rules_path = rules;
          }) {
        remove_files();
    }
    ~ScratchPaths() { remove_files(); }

    ScratchPaths(const ScratchPaths&) = delete;
    ScratchPaths& operator=(const ScratchPaths&) = delete;

private:
    void remove_files() const {
        std::remove(ini.c_str());
        std::remove(db.c_str());
        std::remove(rules.c_str());
    }
};

#endif  // HYDRA_TESTS_SCRATCH_PATHS_H
