# Hydra — Windows-only C++ build. Produces:
#   * Hydra.exe                       — the GUI
#   * hydra_batch / hydra_fillcompare — the command line tools
#   * hydra_tests                     — the doctest harness
#   * hydra_uitest                    — the headless GUI test runner
#   * hydra_bench / hydra_replay      — developer tools (build_cpp.ps1 builds them)
#   * a zip release via CPack (build_cpp.ps1 -Package)
#
# Third-party code is vendored under third_party/ and built here, so there is
# nothing to install but a compiler + CMake. THIRD_PARTY_NOTICES.txt lists
# every library that ships, with its licence; doctest is vendored for the
# tests only.
#
# Configure/build (CMake and MSVC come from Visual Studio; see build_cpp.ps1):
#   cmake --preset default
#   cmake --build --preset default

cmake_minimum_required(VERSION 3.21)
# The single source of the version: the installer and the HTTP User-Agent
# (via src/core/version.h) both take it from here.
project(Hydra VERSION 2.3.0 LANGUAGES C CXX RC)
# The single source of the name the app shows (the window title, the Start
# Menu shortcut, the installer) and of its taskbar identity (AppUserModelID).
# The identity keeps its old value so existing taskbar pins still group with
# the running window. src/core/version.h and installer/build_installer.ps1
# both take them from these two lines.
set(HYDRA_APP_NAME "Hydra Deluxe")
set(HYDRA_APP_USER_MODEL_ID "Hydra.Hydra")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    message(FATAL_ERROR "Hydra's C++ port is Windows/MSVC only.")
endif()

# Parallel compilation within each target.
add_compile_options(/MP)

# Release builds: link-time optimization in the installer's build where the
# toolchain supports it, and a symbol file (.pdb) beside each exe for
# profiling and crash dumps. The install() rules below name no .pdb, so none
# ships; the installer checks that. docs/development.md says why the riskier
# speed flags stay out, and why the everyday build leaves link-time
# optimization off.
option(HYDRA_LTCG "Link-time optimization for Release (the ship preset turns it on)" OFF)
if(HYDRA_LTCG)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT HYDRA_IPO_SUPPORTED OUTPUT HYDRA_IPO_OUTPUT LANGUAGES C CXX)
endif()
if(HYDRA_LTCG AND HYDRA_IPO_SUPPORTED)
    message(STATUS "Link-time optimization (IPO): supported, on for Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    # Plain /LTCG, never the incremental kind. The Visual Studio generator only
    # sets WholeProgramOptimization, and MSBuild turns that into
    # /LTCG:incremental, whose cache files (.iobj, .ipdb) went stale and broke
    # links or tests (CMake issue 20484; docs/development.md).
    add_link_options($<$<CONFIG:Release>:/LTCG>)
elseif(HYDRA_LTCG)
    message(STATUS "Link-time optimization (IPO): not supported, off: ${HYDRA_IPO_OUTPUT}")
else()
    message(STATUS "Link-time optimization (IPO): off (HYDRA_LTCG is OFF; the ship preset turns it on)")
endif()
add_compile_options($<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/Zi>)
# /DEBUG turns off /OPT:REF and /OPT:ICF unless they are named, so they are
# named to keep the exes as lean as before. /PDBALTPATH stores only the .pdb's
# file name in the exe, not the build folder's path, which the installer's
# "no repo path" check would refuse.
add_link_options($<$<CONFIG:Release>:/DEBUG>
                 $<$<CONFIG:Release>:/OPT:REF>
                 $<$<CONFIG:Release>:/OPT:ICF>
                 $<$<CONFIG:Release>:/PDBALTPATH:%_PDB%>)

# __FILE__ names the source relative to the repo, not by its absolute path.
# Opus's safety checks (OPUS_HARDENING) store __FILE__ for their error message,
# which put the builder's folder path into the shipped Hydra.exe 28 times.
# /d1trimfile strips the prefix (MSVC 16.4+); the checks themselves stay on.
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/" HYDRA_SOURCE_PREFIX)
add_compile_options($<$<COMPILE_LANGUAGE:C,CXX>:/d1trimfile:${HYDRA_SOURCE_PREFIX}>)

# Strict flags for our own code, applied via this interface target; third-party
# gets relaxed warnings so a vendor update never turns into a wall of warnings.
# /utf-8: our sources are UTF-8, and several hold non-ASCII text in string
# literals (em dashes, arrows). Without it MSVC reads them in the system code
# page, which works on an English Windows only by luck. C++ only, so the
# resource compiler never sees it.
# /WX: our code builds with zero warnings, so any new one fails the build
# instead of scrolling past. Third-party targets don't link this.
add_library(hydra_warnings INTERFACE)
target_compile_options(hydra_warnings INTERFACE
    /W4 /WX /EHsc $<$<COMPILE_LANGUAGE:CXX>:/utf-8>)

# ---- third_party: Dear ImGui (core + Win32/DX11 backends) ----------------
add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_win32.cpp
    third_party/imgui/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui PUBLIC
    third_party/imgui
    third_party/imgui/backends
    third_party  # imconfig.h includes "imgui_test_engine/imgui_te_imconfig.h"
)
target_compile_options(imgui PRIVATE /W1 /EHsc)

# ---- third_party: Dear ImGui Test Engine ---------------------------------
# Drives the real GUI by widget label for the hydra_uitest runner and the
# `Hydra.exe --uitest` attached mode (docs/agents/ui-testing.md). imconfig.h
# turns its hooks on for every ImGui build, so this links wherever imgui does.
add_library(imgui_test_engine STATIC
    third_party/imgui_test_engine/imgui_capture_tool.cpp
    third_party/imgui_test_engine/imgui_te_context.cpp
    third_party/imgui_test_engine/imgui_te_coroutine.cpp
    third_party/imgui_test_engine/imgui_te_engine.cpp
    third_party/imgui_test_engine/imgui_te_exporters.cpp
    third_party/imgui_test_engine/imgui_te_perftool.cpp
    third_party/imgui_test_engine/imgui_te_ui.cpp
    third_party/imgui_test_engine/imgui_te_utils.cpp
)
target_include_directories(imgui_test_engine PUBLIC third_party/imgui_test_engine)
target_link_libraries(imgui_test_engine PUBLIC imgui)
target_compile_options(imgui_test_engine PRIVATE /W1 /EHsc)

# ---- third_party: SQLite3 amalgamation -----------------------------------
add_library(sqlite3 STATIC third_party/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC third_party/sqlite)
target_compile_options(sqlite3 PRIVATE /W1)
# Two options from SQLite's compile-time list that trim its own CPU: no
# memory-use counters (Hydra never reads them) and no shared cache (Hydra
# opens its own connections). tests/test_store.cpp checks both are built in.
# The WAL sync default is left alone: record_store.cpp sets it on every open.
target_compile_definitions(sqlite3 PRIVATE SQLITE_DEFAULT_MEMSTATUS=0 SQLITE_OMIT_SHARED_CACHE)

# ---- third_party: miniz (raw deflate, for Clone Hero .srb bundles) -------
add_library(miniz STATIC third_party/miniz/miniz.c)
target_include_directories(miniz PUBLIC third_party/miniz)
target_compile_options(miniz PRIVATE /W1 /EHsc)

# ---- third_party: chart Preview media libraries --------------------------
# Vendored for the 3D note-highway Preview (see docs/adr/0004, 0005):
# miniaudio (public domain/MIT-0), stb_vorbis (public domain), libogg +
# libopus (BSD).
#
# EXCLUDE_FROM_ALL: each of these builds only when a target that links it is
# built. hydra_audio links all four, so only the tools that link hydra_audio
# compile them.
#
# miniaudio drives the output device and converts sample rates and channel
# counts; each audio format is decoded to PCM by dedicated code (stb_vorbis
# for OGG, libopus + libogg for OPUS, miniaudio's own dr_libs for MP3/WAV/FLAC).
add_library(miniaudio STATIC EXCLUDE_FROM_ALL third_party/miniaudio/miniaudio.c)
target_include_directories(miniaudio PUBLIC third_party/miniaudio)
target_compile_options(miniaudio PRIVATE /W1)
# Compile out what Hydra never calls (option names from miniaudio.h's own
# table): encoders, generators, the high-level engine, its node graph and
# resource manager, and every backend but WASAPI. PUBLIC so every file that
# includes miniaudio.h sees the same declarations as the implementation.
target_compile_definitions(miniaudio PUBLIC
    MA_NO_ENCODING
    MA_NO_GENERATION
    MA_NO_ENGINE
    MA_NO_NODE_GRAPH
    MA_NO_RESOURCE_MANAGER
    MA_ENABLE_ONLY_SPECIFIC_BACKENDS
    MA_ENABLE_WASAPI
)

add_library(stb_vorbis STATIC EXCLUDE_FROM_ALL third_party/stb/stb_vorbis.c)
target_include_directories(stb_vorbis PUBLIC third_party/stb)
target_compile_options(stb_vorbis PRIVATE /W1)

# libogg + libopus build through their own CMake. Force static libraries and
# drop their tests, programs, and docs so only the codec libs are produced.
# libogg's cmake_minimum_required is 2.8.12; CMake 4.x refuses compatibility
# below 3.5, so raise the effective floor for these vendored subprojects. This
# does not touch the vendored files, and our own top-level minimum is 3.21.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/ogg EXCLUDE_FROM_ALL)
add_subdirectory(third_party/opus EXCLUDE_FROM_ALL)

# ---- third_party: mimalloc (the memory allocator every exe runs on) --------
# D88 ships Microsoft's mimalloc as its override DLL: mimalloc.dll replaces
# malloc/free for the whole process, helped by the prebuilt
# mimalloc-redirect.dll. These options are the ones the trial measured
# (docs/handoffs/2026-10-07-mimalloc-trial.md); everything else stays at
# mimalloc's release defaults. hydra_use_mimalloc (bottom of this file) owns
# which exes link it and how.
set(MI_BUILD_SHARED ON CACHE BOOL "" FORCE)
set(MI_OVERRIDE ON CACHE BOOL "" FORCE)
set(MI_WIN_REDIRECT ON CACHE BOOL "" FORCE)
set(MI_BUILD_STATIC OFF CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# mimalloc's CMake compiles `git describe` into the DLL whenever the top-level
# source folder holds a .git folder, and then reconfigures on every change to
# the git index. The measured build came from the source tarball, which has
# no .git, so Git stays hidden from it here.
set(CMAKE_DISABLE_FIND_PACKAGE_Git TRUE)
add_subdirectory(third_party/mimalloc EXCLUDE_FROM_ALL)
unset(CMAKE_DISABLE_FIND_PACKAGE_Git)
# mimalloc takes HYDRA_LTCG like every target, so the installer's DLL is
# link-time optimized, which the trial did not measure (user decision D99).

# ---- hydra_image: the single stb_image TU (shared PNG/JPEG decode) --------
# stb_image's implementation may be compiled in exactly one TU across the binary
# (it defines non-static symbols), yet both the GUI icons and the Preview
# renderer's textures decode images. So the STB_IMAGE_IMPLEMENTATION lives
# here behind image/decode.h and both link this one lib.
add_library(hydra_image STATIC src/image/decode.cpp)
target_include_directories(hydra_image PUBLIC src PRIVATE third_party/stb)
# stb_image.h is third-party; keep it out of the /W4 sweep. /utf-8 as for
# every Hydra target (see hydra_warnings).
target_compile_options(hydra_image PRIVATE /W1 /EHsc /utf-8)

# ---- hydra_core: everything below the GUI/CLI shells ---------------------
add_library(hydra_core STATIC
    src/core/timing.cpp
    src/core/model.cpp
    src/core/scoring.cpp
    src/core/rules.cpp
    src/core/squeeze_rating.cpp
    src/core/stars.cpp
    src/core/replay.cpp
    src/core/strutil.cpp
    src/core/winstr.cpp
    src/core/audio_sniff.cpp
    src/parse/midi.cpp
    src/parse/song.cpp
    src/parse/srb.cpp
    src/parse/chart_files.cpp
    src/parse/sng.cpp
    src/search/graph.cpp
    src/search/engine.cpp
    src/search/pather.cpp
    src/store/record_store.cpp
    src/store/upgrade_files.cpp
    src/app/allocator.cpp
    src/app/analysis.cpp
    src/app/config.cpp
    src/app/rules_file.cpp
    src/app/html_page.cpp
    src/app/dynamics_breakdown.cpp
    src/app/display_format.cpp
    src/app/library_query.cpp
    src/app/path_view.cpp
    src/app/preview_view.cpp
    src/app/preview_source.cpp
    src/app/song_length.cpp
    src/app/fill_report.cpp
    src/app/report.cpp
    src/app/report_view.cpp
    src/app/path_report_view.cpp
    src/app/report_files.cpp
    src/app/user_messages.cpp
)
target_include_directories(hydra_core PUBLIC src)
target_compile_definitions(hydra_core PUBLIC HYDRA_VERSION="${PROJECT_VERSION}"
                           HYDRA_APP_NAME="${HYDRA_APP_NAME}"
                           HYDRA_APP_USER_MODEL_ID="${HYDRA_APP_USER_MODEL_ID}")
# store/ needs sqlite3; app/ hashes chart files via CNG (bcrypt.lib) and walks
# folders via the Win32 Find*File API (already linked into every Windows exe).
# shell32: report_files.cpp hands hydra_fillcompare's finished page to the
# browser with ShellExecuteW, and winstr.cpp splits every entry point's
# Unicode command line with CommandLineToArgvW. shell32 + ole32 + uuid:
# report_files.cpp finds Documents with SHGetKnownFolderPath(FOLDERID_Documents)
# and frees the answer with CoTaskMemFree. shlwapi: open_in_browser looks up
# the .html program (AssocQueryStringW) to hand it a long page's short name
# directly.
target_link_libraries(hydra_core PUBLIC sqlite3 miniz)
target_link_libraries(hydra_core PRIVATE bcrypt shell32 shlwapi ole32 uuid hydra_warnings)
# allocator.cpp calls mimalloc by name, so it needs mimalloc.h. Only the
# header folder: every exe links mimalloc first already (hydra_use_mimalloc),
# and linking it here too makes CMake move it after hydra_core on each exe's
# link line, behind d3d11.dll, where mimalloc-redirect.dll gives up and leaves
# malloc on the Windows heap.
target_include_directories(hydra_core PRIVATE
                           $<TARGET_PROPERTY:mimalloc,INTERFACE_INCLUDE_DIRECTORIES>)

# ---- hydra_audio: the Preview audio engine (decode + mix + player) -------
# Decodes chart stems to float PCM, mixes them, and (GUI-side) drives the
# output device. It also owns a song's length, the end of its mixed audio
# (song_audio.cpp, D69). Kept separate from hydra_core so hydra_fillcompare
# never pulls in the codec libraries; linked by hydra_ui,
# hydra_tests and hydra_batch (so a batch can save song lengths). The decode and mix
# code is pure and unit-tested; only the player owns a miniaudio device.
# Depends on hydra_core for the PreviewAudioStem type and file I/O helpers.
add_library(hydra_audio STATIC
    src/audio/decode.cpp
    src/audio/mapped_file.cpp
    src/audio/song_audio.cpp
    src/audio/stem_reader.cpp
    src/audio/opus_reader.cpp
    src/audio/vorbis_reader.cpp
    src/audio/ma_reader.cpp
    src/audio/mixer.cpp
    src/audio/stream_mix.cpp
    src/audio/player.cpp
    src/audio/device.cpp
)
target_link_libraries(hydra_audio PUBLIC hydra_core)
# miniaudio decodes WAV/MP3/FLAC (its dr_libs) and, GUI-side, drives the device;
# stb_vorbis decodes OGG; libopus over a libogg page demux decodes OPUS.
target_link_libraries(hydra_audio PRIVATE
    miniaudio stb_vorbis opus ogg hydra_warnings)

# ---- hydra_render: the Preview 3D note-highway renderer (GUI-side) --------
# A faithful port of Onyx's 3D drum previewer
# (docs/adr/0008-preview-is-a-faithful-port-of-onyx.md) rendering to an
# offscreen D3D11 target (docs/adr/0005). The pure half — the .obj reader, the
# config, the instant timeline (track_state) and the draw-list builder
# (highway_draw, where every Onyx number lives) — is device-free and
# unit-tested; only preview_renderer touches D3D. Kept out of hydra_core like
# hydra_audio; linked by hydra_ui and hydra_tests.
add_library(hydra_render STATIC
    src/render/obj_loader.cpp
    src/render/preview_config.cpp
    src/render/track_state.cpp
    src/render/highway_draw.cpp
    src/render/overlay_layout.cpp
    src/render/preview_renderer.cpp
)
target_link_libraries(hydra_render PUBLIC hydra_core)
# third_party/json parses assets/preview/3d-config.json (preview_config).
target_include_directories(hydra_render PRIVATE third_party/json)
# d3d11/dxgi drive the offscreen targets and d3dcompiler compiles the HLSL
# files at runtime; hydra_image decodes the Onyx textures.
target_link_libraries(hydra_render PRIVATE
    d3d11 dxgi d3dcompiler hydra_image hydra_warnings)

# ---- hydra_dm: the DMBot leaderboards feature (GUI-only) -----------------
# Kept out of hydra_core so the CLI tools don't link WinHTTP or carry the
# comparison page. third_party/json parses the leaderboard API responses.
add_library(hydra_dm STATIC
    src/app/dm_report.cpp
    src/app/dm_report_view.cpp
    src/net/dmbot_client.cpp
)
target_link_libraries(hydra_dm PUBLIC hydra_core)
target_link_libraries(hydra_dm PRIVATE winhttp hydra_warnings)
target_include_directories(hydra_dm PRIVATE third_party/json)

# ---- hydra_ui: the window-independent GUI ---------------------------------
# Everything but the Win32 window (main.cpp), shared by Hydra.exe and the
# headless GUI test runner.
add_library(hydra_ui STATIC
    src/ui/app_shell.cpp
    src/ui/app_state.cpp
    src/ui/details_panel.cpp
    src/ui/dm_jobs.cpp
    src/ui/dynamics_tab.cpp
    src/ui/icons.cpp
    src/ui/imgui_files.cpp  # ImGui's file functions (imconfig.h)
    src/ui/paths_tab.cpp
    src/ui/preview_tab.cpp
    src/ui/stars_tab.cpp
    src/ui/library_dialogs.cpp
    src/ui/library_jobs.cpp
    src/ui/library_model.cpp
    src/ui/library_table.cpp
    src/ui/library_toolbar.cpp
    src/ui/settings_bar.cpp
    src/ui/library_view.cpp
    src/ui/preview_controller.cpp
    src/ui/preview_load_job.cpp
    src/ui/preview_transport.cpp
    src/ui/report_window.cpp
    src/ui/path_report_window.cpp
    src/ui/dm_report_window.cpp
    src/ui/theme.cpp
    src/ui/win32_dialogs.cpp
)
# hydra_render + hydra_audio join the GUI here (Phase 5): the Preview tab's
# controller (preview_controller.cpp) drives the offscreen renderer and the
# audio engine, and preview_load_job.cpp's PreviewLoadJob decodes + mixes
# stems. PUBLIC so the exes link them transitively.
target_link_libraries(hydra_ui PUBLIC hydra_dm imgui imgui_test_engine hydra_render hydra_audio)
# ole32: the folder-picker dialog (COM's IFileOpenDialog). shell32 is not
# needed here: hydra_core links it for report_files.cpp's ShellExecuteW.
# hydra_image: icons.cpp decodes its PNGs through the shared stb TU.
target_link_libraries(hydra_ui PRIVATE ole32 hydra_image hydra_warnings)

# Attached GUI testing (`Hydra.exe --uitest ...`, docs/agents/ui-testing.md)
# builds the GUI tests into the exe. On for dev builds; the "ship" preset (the
# installer's build) turns it off, so the shipped exe carries no tests and no
# repo paths.
option(HYDRA_UITEST_ATTACHED
       "Build Hydra.exe with the attached GUI test harness (off for the installer)" ON)

# ---- hydra_uitest_harness: the GUI tests, compiled once --------------------
# The harness, the script runner and the tests. hydra_uitest always links
# them; Hydra.exe links them when HYDRA_UITEST_ATTACHED is on. The repo paths
# the tests need are compiled in here and nowhere else.
add_library(hydra_uitest_harness STATIC
    tests/ui/uitest_harness.cpp
    tests/ui/uitest_script.cpp
    tests/ui/uitest_tests.cpp
    tests/ui/uitest_library.cpp
    tests/ui/uitest_details.cpp
    tests/ui/uitest_preview.cpp
    tests/ui/uitest_batch_reports.cpp
    tests/ui/uitest_paths.cpp
    tests/ui/uitest_report_windows.cpp
    tests/report_samples.cpp
)
target_include_directories(hydra_uitest_harness PUBLIC tests/ui)
target_link_libraries(hydra_uitest_harness PUBLIC hydra_ui PRIVATE hydra_warnings)
target_compile_definitions(hydra_uitest_harness PRIVATE
    HYDRA_INPUT_DIR="${CMAKE_SOURCE_DIR}/testdata/input"
    HYDRA_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets/preview"
    HYDRA_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/resource"
)

# ---- Hydra.exe: the GUI shell ---------------------------------------------
# The SP cap (once a separate "Uncapped" build) is a runtime setting, so there
# is one exe.
add_executable(Hydra WIN32 src/ui/main.cpp src/ui/hydra.rc)
target_link_libraries(Hydra PRIVATE hydra_ui d3d11 dxgi d3dcompiler hydra_warnings)
# GUI-subsystem app entered from a standard int main(), like the ImGui example.
target_link_options(Hydra PRIVATE /ENTRY:mainCRTStartup)
# resource/ (fonts, icons) is read relative to the exe at runtime; keep a
# copy beside the build output so the exe doesn't need to run from the repo
# root.
add_custom_command(TARGET Hydra POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:Hydra>/resource"
)
# assets/ (the Preview highway art, loaded at runtime via exe_dir()) is
# staged beside the exe the same way resource/ is (see assets/preview/README).
add_custom_command(TARGET Hydra POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:Hydra>/assets"
)

# Attached GUI testing: a test runs inside the real window at watchable speed.
# main.cpp compiles the --uitest path only under HYDRA_UITEST_ATTACHED.
if(HYDRA_UITEST_ATTACHED)
    target_link_libraries(Hydra PRIVATE hydra_uitest_harness)
    target_compile_definitions(Hydra PRIVATE HYDRA_UITEST_ATTACHED)
endif()

# ---- hydra_batch / hydra_fillcompare: the command-line tools -------------
# Both read the same settings INI and database the GUI uses.
add_executable(hydra_batch src/cli/batch.cpp)
add_executable(hydra_fillcompare src/cli/fillcompare.cpp)
foreach(tool IN ITEMS hydra_batch hydra_fillcompare)
    target_link_libraries(${tool} PRIVATE hydra_core hydra_warnings)
endforeach()

# ---- hydra_tests: the doctest harness ------------------------------------
add_executable(hydra_tests
    tests/test_main.cpp
    tests/test_strutil.cpp
    tests/test_library_query.cpp
    tests/test_corpus_cache.cpp
    tests/test_timing.cpp
    tests/test_chord_code.cpp
    tests/test_winstr.cpp
    tests/test_midi.cpp
    tests/test_model.cpp
    tests/test_squeeze_rating.cpp
    tests/test_stars.cpp
    tests/test_theme.cpp
    tests/test_batch_text.cpp
    tests/test_display_format.cpp
    tests/test_path_view.cpp
    tests/test_preview_view.cpp
    tests/test_preview_source.cpp
    tests/test_audio_decode.cpp
    tests/test_stem_reader.cpp
    tests/test_audio_mixer.cpp
    tests/test_stream_mix.cpp
    tests/test_audio_player.cpp
    tests/test_audio_device.cpp
    tests/test_obj_loader.cpp
    tests/test_preview_config.cpp
    tests/test_track_state.cpp
    tests/test_highway_draw.cpp
    tests/test_overlay_layout.cpp
    tests/test_preview_golden.cpp
    tests/test_preview_clock.cpp
    tests/test_preview_transport.cpp
    tests/test_image_decode.cpp
    tests/test_preview_renderer.cpp
    tests/test_dynamics_breakdown.cpp
    tests/test_song.cpp
    tests/test_sng.cpp
    tests/test_srb.cpp
    tests/test_search.cpp
    tests/test_fast_tempo.cpp
    tests/test_replay.cpp
    tests/test_store.cpp
    tests/test_upgrade_files.cpp
    tests/test_analysis.cpp
    tests/test_report.cpp
    tests/report_samples.cpp
    tests/test_report_view.cpp
    tests/test_config.cpp
    tests/test_rules.cpp
    tests/test_dm_report.cpp
    tests/test_user_messages.cpp
    tests/test_library_jobs.cpp
    tests/test_fill_deadline.cpp
    tests/test_fill_report.cpp
    tests/test_app_state.cpp
    tests/test_song_panel_state.cpp
    tests/test_library_model.cpp
    tests/test_app_shell.cpp
    tests/test_label_layout.cpp
    tests/test_activation_row_layout.cpp
    tests/test_preview_controller.cpp
    tests/test_preview_load_progress.cpp
    tests/test_song_length.cpp
    tests/test_utf8_paths.cpp
    tests/test_long_paths.cpp
    tests/test_single_owner.cpp
    tests/test_docs_match_code.cpp
    tests/test_s2_difficulty_table.cpp
    tests/test_s2_disco.cpp
    tests/test_s2_kick2x.cpp
    tests/test_s2_phrase_end.cpp
    tests/test_s2_fill_landing.cpp
    tests/test_s2_dynamics_tag.cpp
    tests/test_s2_parser_owners.cpp
    tests/test_s2_offspeed.cpp
    tests/test_s2_deact_extension.cpp
    tests/test_perf_digest.cpp
)
# hydra_dm: test_dm_report exercises the leaderboards join and JSON parsers.
# hydra_audio: test_audio_decode exercises the stem decoders and mixer.
# hydra_render: the highway geometry, meshes, and a WARP offscreen-render test —
# hence d3d11/dxgi (a headless WARP device) and d3dcompiler (runtime HLSL).
# hydra_ui: test_app_state builds a real AppState on a scratch store. Only the
# window-independent half is exercised; nothing here opens a window or a device.
target_link_libraries(hydra_tests PRIVATE
    hydra_ui hydra_dm hydra_audio hydra_render hydra_image d3d11 dxgi d3dcompiler
    hydra_warnings)
# test_stem_reader keeps copies of the old whole-file decoders as its reference
# and re-pages Opus fixtures with libogg, so it needs the codec headers too.
target_link_libraries(hydra_tests PRIVATE miniaudio stb_vorbis opus ogg)
target_include_directories(hydra_tests PRIVATE
    third_party/doctest
    third_party/json
)
# doctest.h specializes std::tuple, which newer MSVC flags (C5285); harmless.
target_compile_options(hydra_tests PRIVATE /wd5285)

# Where the chart corpus and fixtures live, so tests find them regardless of
# the working directory.
target_compile_definitions(hydra_tests PRIVATE
    HYDRA_TESTDATA_DIR="${CMAKE_SOURCE_DIR}/testdata"
    HYDRA_INPUT_DIR="${CMAKE_SOURCE_DIR}/testdata/input"
    HYDRA_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets/preview"
    HYDRA_SOURCE_DIR="${CMAKE_SOURCE_DIR}"  # test_single_owner scans src/ and tools/
)

# ---- hydra_uitest: the headless GUI test runner ----------------------------
# Runs the real ImGui UI with no window, driven by the Dear ImGui Test Engine;
# see docs/agents/ui-testing.md. The GUI tests themselves live in
# hydra_uitest_harness (above), which Hydra.exe shares in dev builds.
add_executable(hydra_uitest tests/ui/uitest_main.cpp)
target_link_libraries(hydra_uitest PRIVATE
    hydra_uitest_harness d3d11 dxgi d3dcompiler hydra_warnings)

enable_testing()
add_test(NAME hydra_tests COMMAND hydra_tests)
add_test(NAME hydra_uitest COMMAND hydra_uitest --all)

# ---- packaging -----------------------------------------------------------
# `cpack -G ZIP` (or build_cpp.ps1 -Package) produces Hydra-<ver>-win64.zip:
# everything the install() rules below stage, all in one folder.
# User data (hydra*.db/.ini) is created next to the exe at runtime and never
# lives in the package, so a rebuilt zip can't clobber anyone's records.
install(TARGETS Hydra hydra_batch hydra_fillcompare RUNTIME DESTINATION .)
# Every exe above needs mimalloc's two DLLs beside it to start (see
# hydra_use_mimalloc). install(FILES) rather than install(TARGETS), so
# mimalloc's import library stays out of the stage.
install(FILES $<TARGET_FILE:mimalloc>
              "${CMAKE_SOURCE_DIR}/third_party/mimalloc/bin/mimalloc-redirect.dll"
        DESTINATION .)
install(DIRECTORY resource/ DESTINATION resource)
install(DIRECTORY assets/ DESTINATION assets)
# THIRD_PARTY_NOTICES.txt carries the licence of every vendored library
# compiled into a shipped exe or DLL.
install(FILES LICENSE THIRD_PARTY_NOTICES.txt README.md DESTINATION .)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_FILE_NAME "Hydra-${PROJECT_VERSION}-win64")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")
include(CPack)

# ---- hydra_bench: engine timing harness (not a test; built on demand) ----
add_executable(hydra_bench EXCLUDE_FROM_ALL tools/bench.cpp)
target_link_libraries(hydra_bench PRIVATE hydra_core hydra_warnings)
target_include_directories(hydra_bench PRIVATE tests third_party/json)
target_compile_definitions(hydra_bench PRIVATE
    HYDRA_INPUT_DIR="${CMAKE_SOURCE_DIR}/testdata/input"
)

# ---- hydra_replay: arbitrary-path scorer (not a test; built on demand) ----
# Prices a hand-written Star Power path chord by chord, dumps a record's
# stored paths, and self-checks the replay against the engine over the
# corpus (hence the same corpus_util.h wiring hydra_bench uses).
add_executable(hydra_replay EXCLUDE_FROM_ALL tools/replay.cpp)
target_link_libraries(hydra_replay PRIVATE hydra_core hydra_warnings)
target_include_directories(hydra_replay PRIVATE tests third_party/json)
target_compile_definitions(hydra_replay PRIVATE
    HYDRA_INPUT_DIR="${CMAKE_SOURCE_DIR}/testdata/input"
)

# ---- CLI end-to-end tests ------------------------------------------------
# tests/test_cli.cpp runs the two console tools as real processes, so
# building the tests builds the tools first and tells the tests where they are.
target_sources(hydra_tests PRIVATE tests/test_cli.cpp)
add_dependencies(hydra_tests hydra_batch hydra_fillcompare)
target_compile_definitions(hydra_tests PRIVATE
    HYDRA_BATCH_EXE="$<TARGET_FILE:hydra_batch>"
    HYDRA_FILLCOMPARE_EXE="$<TARGET_FILE:hydra_fillcompare>"
)

# ---- hydra_replay_json: the dump/target JSON shapes (tools only) ----------
# hydra_replay writes and reads these, and tests/test_replay pins them. The
# app never reads a path as JSON, so this stays out of hydra_core, and core/
# stays free of the JSON library.
add_library(hydra_replay_json STATIC tools/replay_json.cpp)
target_include_directories(hydra_replay_json PUBLIC tools third_party/json)
target_link_libraries(hydra_replay_json PUBLIC hydra_core PRIVATE hydra_warnings)
target_link_libraries(hydra_replay PRIVATE hydra_replay_json)
target_link_libraries(hydra_tests PRIVATE hydra_replay_json)

# ---- long-path awareness, for every executable ---------------------------
# One manifest, merged into each exe by the linker, declares Hydra long-path
# aware (see src/app/long_paths.manifest). Hydra's own file calls handle long
# paths through core/winstr regardless; this lets third-party code do the same
# on machines with long paths enabled.
#
# hydra_tests is left out on purpose. Without the manifest Windows refuses a
# plain long path even where long paths are enabled, so test_long_paths proves
# Hydra's own handling rather than the machine's setting.
foreach(hydra_exe Hydra hydra_batch hydra_fillcompare
                  hydra_uitest hydra_bench hydra_replay)
    target_sources(${hydra_exe} PRIVATE src/app/long_paths.manifest)
endforeach()

# ---- mimalloc, for every executable (D88) ---------------------------------
# The one place that decides which exes run on mimalloc and how they load it.
# It takes every executable defined in this file, so the tests, the GUI test
# runner and the benchmarks run on the same allocator that ships, and a new
# exe joins without anyone remembering to add it. It must stay the last thing
# in the file, after every add_executable.
#
# /include:mi_version is the switch mimalloc's readme gives to make sure the
# exe really imports mimalloc.dll, even an exe that never calls mimalloc by
# name (src/app/allocator.h has the calls Hydra does make).
# mimalloc goes first in each exe's link libraries because the readme
# (third_party/mimalloc/bin/readme.md, step 4) wants mimalloc.dll as early as
# possible in the import list, so it is in place before any other DLL
# allocates; the linker lists imports in library order, and an appended
# mimalloc came after d3d11.dll in Hydra.exe.
# The DLL is built straight into the exes' output folder, where mimalloc's
# own post-build step copies mimalloc-redirect.dll beside it, so every exe
# starts from the build folder. tests/test_cli.cpp copies the CLI tools out of
# that folder together with every DLL beside them.
function(hydra_use_mimalloc)
    set_target_properties(mimalloc PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    get_property(targets DIRECTORY "${CMAKE_SOURCE_DIR}"
                 PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(type ${target} TYPE)
        if(type STREQUAL "EXECUTABLE")
            get_target_property(libs ${target} LINK_LIBRARIES)
            if(NOT libs)
                set(libs "")
            endif()
            set_property(TARGET ${target} PROPERTY LINK_LIBRARIES mimalloc ${libs})
            target_link_options(${target} PRIVATE /include:mi_version)
        endif()
    endforeach()
endfunction()
hydra_use_mimalloc()
