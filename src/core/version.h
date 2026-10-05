// The app version, name and taskbar identity, single-sourced from
// CMakeLists.txt via target_compile_definitions. The HTTP User-Agent uses the
// version; the window and the taskbar use the name and the identity. The
// installer reads the same three strings from CMakeLists.txt, so a release
// bump or a rename is one edit there. Stored results are stamped separately
// (src/store/stored_versions.h).

#ifndef HYDRA_CORE_VERSION_H
#define HYDRA_CORE_VERSION_H

#ifndef HYDRA_VERSION
#error "HYDRA_VERSION must be defined by the build (see CMakeLists.txt)"
#endif
#ifndef HYDRA_APP_NAME
#error "HYDRA_APP_NAME must be defined by the build (see CMakeLists.txt)"
#endif
#ifndef HYDRA_APP_USER_MODEL_ID
#error "HYDRA_APP_USER_MODEL_ID must be defined by the build (see CMakeLists.txt)"
#endif

// The same version as a wide literal (L"1.4.1"-style).
#define HYDRA_WIDEN_(s) L##s
#define HYDRA_WIDEN(s) HYDRA_WIDEN_(s)
#define HYDRA_VERSION_W HYDRA_WIDEN(HYDRA_VERSION)

namespace hydra {
// Window title and the taskbar identity (AppUserModelID). The title is the
// bare name, with no version. The AppUserModelID keeps its old value so
// existing taskbar pins still group with the running window; the installer's
// Start Menu shortcut carries the same one.
inline constexpr const wchar_t* kWindowTitleW = HYDRA_WIDEN(HYDRA_APP_NAME);
inline constexpr const wchar_t* kAppUserModelIDW = HYDRA_WIDEN(HYDRA_APP_USER_MODEL_ID);
}  // namespace hydra

#endif  // HYDRA_CORE_VERSION_H
