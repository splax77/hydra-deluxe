// Tests for what the build settings in CMakeLists.txt leave inside the exes.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <string>

namespace {

// The CodeView record a linker writes for /DEBUG: "RSDS", the .pdb's GUID and
// age, then the .pdb's path as a NUL-terminated string.
struct CodeViewRsds {
    std::uint32_t signature;
    GUID guid;
    std::uint32_t age;
    char path[1];
};

// The .pdb path this exe's debug directory stores, or "" when it has none.
std::string stored_pdb_path() {
    const auto* base = reinterpret_cast<const std::uint8_t*>(GetModuleHandleW(nullptr));
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const IMAGE_DATA_DIRECTORY& dir =
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    const auto* entries = reinterpret_cast<const IMAGE_DEBUG_DIRECTORY*>(base + dir.VirtualAddress);
    const std::size_t count = dir.Size / sizeof(IMAGE_DEBUG_DIRECTORY);
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i].Type != IMAGE_DEBUG_TYPE_CODEVIEW || entries[i].AddressOfRawData == 0)
            continue;
        const auto* cv = reinterpret_cast<const CodeViewRsds*>(base + entries[i].AddressOfRawData);
        if (std::memcmp(&cv->signature, "RSDS", 4) == 0) return cv->path;
    }
    return "";
}

}  // namespace

// /PDBALTPATH in CMakeLists.txt makes each Release exe store only its .pdb's
// file name, which a debugger, the ASan symbolizer and the coverage tool look
// for beside the exe. Given as the linker's %_PDB%, the Visual Studio
// generator mangled it to "%%%hydra_tests.pdb%%%" (CMake issue 15865). Debug
// builds keep the full path, and the coverage tool rewrites the name to the
// full path of its own hydra_tests.instr.pdb, so neither has anything to check.
TEST_CASE("a Release exe stores its .pdb's bare file name") {
#if defined(NDEBUG) && !defined(HYDRA_COVERAGE)
    CHECK(stored_pdb_path() == "hydra_tests.pdb");
#else
    MESSAGE("Debug or coverage build: the stored .pdb name is not the linker's");
#endif
}
