// The Preview's numbers as the app reads them: the shipped
// assets/preview/3d-config.json, loaded by load_preview_config. The json is
// the one source of those numbers (docs/adr/0008), and PreviewConfig carries
// none of its own, so every test that needs a config reads it from here.

#ifndef HYDRA_TESTS_PREVIEW_CONFIG_UTIL_H
#define HYDRA_TESTS_PREVIEW_CONFIG_UTIL_H

#include <string>

#include "core/winstr.h"
#include "render/preview_config.h"

#ifndef HYDRA_ASSET_DIR
#error "HYDRA_ASSET_DIR must be defined (see CMakeLists.txt)"
#endif

// The text of the shipped 3d-config.json.
inline std::string shipped_preview_config_text() {
    return hydra::read_file_text(std::string(HYDRA_ASSET_DIR) + "/3d-config.json");
}

// The shipped 3d-config.json, loaded once.
inline const hydra::render::PreviewConfig& shipped_preview_config() {
    static const hydra::render::PreviewConfig cfg =
        hydra::render::load_preview_config(shipped_preview_config_text());
    return cfg;
}

#endif  // HYDRA_TESTS_PREVIEW_CONFIG_UTIL_H
